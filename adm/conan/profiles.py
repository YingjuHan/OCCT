from __future__ import annotations

from pathlib import Path
from typing import Iterable, List, Set


def resolve_profile_path(root_dir: Path, profile: str) -> Path:
    candidate = Path(profile)
    if candidate.is_file():
        return candidate
    return root_dir / "conan" / "profiles" / profile


def _expand_includes(lines: Iterable[str], base_dir: Path, seen: Set[Path]) -> List[str]:
    expanded: List[str] = []
    for line in lines:
        stripped = line.strip()
        if stripped.startswith("include(") and stripped.endswith(")"):
            include_target = stripped[len("include(") : -1].strip()
            include_path = (base_dir / include_target).resolve()
            if include_path in seen:
                raise ValueError(f"Profile include loop: {include_path}")
            seen.add(include_path)
            include_lines = include_path.read_text(encoding="utf-8").splitlines()
            expanded.extend(_expand_includes(include_lines, include_path.parent, seen))
            continue
        expanded.append(line)
    return expanded


def normalize_profile(profile_path: Path, output_path: Path) -> None:
    lines = profile_path.read_text(encoding="utf-8").splitlines()
    expanded = _expand_includes(lines, profile_path.parent, {profile_path.resolve()})
    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text("\n".join(expanded) + "\n", encoding="utf-8")
