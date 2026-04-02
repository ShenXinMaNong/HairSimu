#!/usr/bin/env python3
"""Analyze hair shading comparison CSVs and generate plots.

Usage:
    /usr/local/bin/python3 scripts/analyze_shading.py output_final/comparison.csv --out analysis

Outputs:
    - metrics.csv
    - average_luminance.png
    - luminance_boxplot.png
    - average_rgb.png
    - scattering_components.png (if component columns exist)
    - report.md
"""

from __future__ import annotations

import argparse
import csv
import math
from collections import defaultdict
from dataclasses import dataclass
from pathlib import Path
from statistics import mean, pstdev
from typing import Dict, Iterable, List, Sequence

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np


@dataclass
class ModelStats:
    name: str
    count: int
    avg_luminance: float
    std_luminance: float
    min_luminance: float
    max_luminance: float
    p10_luminance: float
    p90_luminance: float
    contrast_score: float
    avg_r: float
    avg_g: float
    avg_b: float
    avg_component_r: float | None = None
    avg_component_tt: float | None = None
    avg_component_trt: float | None = None


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Analyze hair shading CSV output and generate comparison plots.")
    parser.add_argument("csv_path", type=Path, help="Path to comparison.csv")
    parser.add_argument("--out", type=Path, default=Path("analysis"), help="Directory to write plots and summaries")
    return parser.parse_args()


def percentile(values: Sequence[float], q: float) -> float:
    if not values:
        return 0.0
    return float(np.percentile(np.asarray(values, dtype=float), q))


def load_rows(csv_path: Path) -> list[dict[str, str]]:
    with csv_path.open(newline="", encoding="utf-8") as f:
        return list(csv.DictReader(f))


def group_by_model(rows: Iterable[dict[str, str]]) -> dict[str, list[dict[str, str]]]:
    grouped: dict[str, list[dict[str, str]]] = defaultdict(list)
    for row in rows:
        grouped[row["model"]].append(row)
    return grouped


def f(row: dict[str, str], key: str, default: float = 0.0) -> float:
    value = row.get(key, "")
    if value == "":
        return default
    try:
        return float(value)
    except ValueError:
        return default


def compute_stats(model: str, rows: list[dict[str, str]]) -> ModelStats:
    luminances = [f(r, "luminance") for r in rows]
    rs = [f(r, "r") for r in rows]
    gs = [f(r, "g") for r in rows]
    bs = [f(r, "b") for r in rows]

    component_r_vals = [f(r, "R") for r in rows] if any("R" in r for r in rows) else []
    component_tt_vals = [f(r, "TT") for r in rows] if any("TT" in r for r in rows) else []
    component_trt_vals = [f(r, "TRT") for r in rows] if any("TRT" in r for r in rows) else []

    has_components = any(v > 0.0 for v in component_r_vals + component_tt_vals + component_trt_vals)

    return ModelStats(
        name=model,
        count=len(rows),
        avg_luminance=mean(luminances) if luminances else 0.0,
        std_luminance=pstdev(luminances) if len(luminances) > 1 else 0.0,
        min_luminance=min(luminances) if luminances else 0.0,
        max_luminance=max(luminances) if luminances else 0.0,
        p10_luminance=percentile(luminances, 10),
        p90_luminance=percentile(luminances, 90),
        contrast_score=percentile(luminances, 90) - percentile(luminances, 10),
        avg_r=mean(rs) if rs else 0.0,
        avg_g=mean(gs) if gs else 0.0,
        avg_b=mean(bs) if bs else 0.0,
        avg_component_r=mean(component_r_vals) if has_components and component_r_vals else None,
        avg_component_tt=mean(component_tt_vals) if has_components and component_tt_vals else None,
        avg_component_trt=mean(component_trt_vals) if has_components and component_trt_vals else None,
    )


def write_metrics_csv(stats: Sequence[ModelStats], out_dir: Path) -> None:
    path = out_dir / "metrics.csv"
    with path.open("w", newline="", encoding="utf-8") as f:
        writer = csv.writer(f)
        writer.writerow([
            "model",
            "count",
            "avg_luminance",
            "std_luminance",
            "min_luminance",
            "max_luminance",
            "p10_luminance",
            "p90_luminance",
                "contrast_score",
            "avg_r",
            "avg_g",
            "avg_b",
            "avg_component_r",
            "avg_component_tt",
            "avg_component_trt",
        ])
        for s in stats:
            writer.writerow([
                s.name,
                s.count,
                f"{s.avg_luminance:.6f}",
                f"{s.std_luminance:.6f}",
                f"{s.min_luminance:.6f}",
                f"{s.max_luminance:.6f}",
                f"{s.p10_luminance:.6f}",
                f"{s.p90_luminance:.6f}",
                f"{s.contrast_score:.6f}",
                f"{s.avg_r:.6f}",
                f"{s.avg_g:.6f}",
                f"{s.avg_b:.6f}",
                "" if s.avg_component_r is None else f"{s.avg_component_r:.6f}",
                "" if s.avg_component_tt is None else f"{s.avg_component_tt:.6f}",
                "" if s.avg_component_trt is None else f"{s.avg_component_trt:.6f}",
            ])


def plot_average_luminance(stats: Sequence[ModelStats], out_dir: Path) -> None:
    names = [s.name for s in stats]
    values = [s.avg_luminance for s in stats]
    errors = [s.std_luminance for s in stats]

    fig, ax = plt.subplots(figsize=(10, 5))
    bars = ax.bar(names, values, yerr=errors, capsize=5, color=["#7f8c8d", "#3498db", "#9b59b6", "#1abc9c", "#e67e22", "#34495e", "#c0392b"])
    ax.set_title("Average Luminance by Shading Model")
    ax.set_ylabel("Luminance")
    ax.set_xticks(range(len(names)))
    ax.set_xticklabels(names, rotation=20, ha="right")
    ax.grid(axis="y", alpha=0.25)
    fig.tight_layout()
    fig.savefig(out_dir / "average_luminance.png", dpi=180)
    plt.close(fig)


def plot_luminance_boxplot(grouped: dict[str, list[dict[str, str]]], out_dir: Path) -> None:
    names = list(grouped.keys())
    data = [[f(r, "luminance") for r in grouped[name]] for name in names]

    fig, ax = plt.subplots(figsize=(10, 5))
    ax.boxplot(data, tick_labels=names, showmeans=True)
    ax.set_title("Luminance Distribution per Model")
    ax.set_ylabel("Luminance")
    ax.tick_params(axis="x", rotation=20)
    ax.grid(axis="y", alpha=0.25)
    fig.tight_layout()
    fig.savefig(out_dir / "luminance_boxplot.png", dpi=180)
    plt.close(fig)


def plot_average_rgb(stats: Sequence[ModelStats], out_dir: Path) -> None:
    names = [s.name for s in stats]
    r = np.array([s.avg_r for s in stats])
    g = np.array([s.avg_g for s in stats])
    b = np.array([s.avg_b for s in stats])

    x = np.arange(len(names))
    width = 0.7
    fig, ax = plt.subplots(figsize=(10, 5))
    ax.bar(x, r, width, label="R", color="#e74c3c")
    ax.bar(x, g, width, bottom=r, label="G", color="#2ecc71")
    ax.bar(x, b, width, bottom=r + g, label="B", color="#3498db")
    ax.set_xticks(x)
    ax.set_xticklabels(names, rotation=20, ha="right")
    ax.set_title("Average RGB Composition by Model")
    ax.set_ylabel("Average channel value")
    ax.legend()
    ax.grid(axis="y", alpha=0.25)
    fig.tight_layout()
    fig.savefig(out_dir / "average_rgb.png", dpi=180)
    plt.close(fig)


def plot_scattering_components(stats: Sequence[ModelStats], out_dir: Path) -> None:
    component_models = [s for s in stats if s.avg_component_r is not None or s.avg_component_tt is not None or s.avg_component_trt is not None]
    if not component_models:
        return

    names = [s.name for s in component_models]
    r = np.array([s.avg_component_r or 0.0 for s in component_models])
    tt = np.array([s.avg_component_tt or 0.0 for s in component_models])
    trt = np.array([s.avg_component_trt or 0.0 for s in component_models])

    x = np.arange(len(names))
    width = 0.65
    fig, ax = plt.subplots(figsize=(10, 5))
    ax.bar(x, r, width, label="R", color="#8e44ad")
    ax.bar(x, tt, width, bottom=r, label="TT", color="#f39c12")
    ax.bar(x, trt, width, bottom=r + tt, label="TRT", color="#16a085")
    ax.set_xticks(x)
    ax.set_xticklabels(names, rotation=20, ha="right")
    ax.set_title("Average Scattering Component Energy")
    ax.set_ylabel("Average component value")
    ax.legend()
    ax.grid(axis="y", alpha=0.25)
    fig.tight_layout()
    fig.savefig(out_dir / "scattering_components.png", dpi=180)
    plt.close(fig)


def write_report(stats: Sequence[ModelStats], csv_path: Path, out_dir: Path) -> None:
    lines = ["# Hair Shading Analysis Report", "", f"Source CSV: `{csv_path}`", "", "## Summary"]
    for s in stats:
        lines.append(
            f"- {s.name}: n={s.count}, avg luminance={s.avg_luminance:.4f}, std={s.std_luminance:.4f}, "
            f"p10={s.p10_luminance:.4f}, p90={s.p90_luminance:.4f}"
        )

    lines += ["", "## Suggested Interpretation", "- Higher average luminance usually indicates stronger overall reflectance.",
               "- Larger std/p90-p10 indicates a wider highlight distribution or more variation across strands.",
               "- R/TT/TRT component plots are most meaningful for physically motivated models."]

    (out_dir / "report.md").write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    args = parse_args()
    args.out.mkdir(parents=True, exist_ok=True)

    rows = load_rows(args.csv_path)
    grouped = group_by_model(rows)
    stats = [compute_stats(model, model_rows) for model, model_rows in grouped.items()]
    stats.sort(key=lambda s: s.name)

    write_metrics_csv(stats, args.out)
    plot_average_luminance(stats, args.out)
    plot_luminance_boxplot(grouped, args.out)
    plot_average_rgb(stats, args.out)
    plot_scattering_components(stats, args.out)
    write_report(stats, args.csv_path, args.out)

    print(f"Wrote analysis to {args.out}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
