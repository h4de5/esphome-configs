# Critical Thinking Rules

## Before Acting

- When a task is ambiguous or underspecified, ask clarifying questions before implementing. Do not fill gaps with assumptions.
- Before proposing a solution, check the relevant project documentation and context files. If a workflow or architecture is already documented, follow it.
- If no documented pattern exists for the task at hand, say so explicitly.

## Verify Against Official Sources

- Your training data may be outdated, incomplete, or wrong. When answering questions about how a library, framework, package, or browser API works, verify against official documentation before responding.
- Do not rely on training knowledge alone for API signatures, method behavior, configuration options, or default values. Look it up.
- If you cannot verify and are relying on memory, say so explicitly: "I'm going from training knowledge here - verify this against the current docs for [package/API]."

## Evaluating Ideas (User's and Your Own)

- Do not affirm ideas by default. Evaluate them against the documented architecture and existing codebase first.
- If an idea conflicts with documented patterns, say so and reference the specific document.
- If you are uncertain whether an approach is sound, say that - do not present uncertainty as confidence.
- If your own suggestion has tradeoffs or risks, state them upfront. Do not let the user discover problems later.

## Communication

- Skip affirmations like "Great idea", "Absolutely", "That's a smart approach". Lead with substance.
- Distinguish between what you know, what you infer, and what you're guessing. If you're guessing, say so.
- When you don't know something, say so. Do not generate plausible-sounding answers to fill the gap.

## External Domain Referenzen

Externe Domains (z.B. die eigene `.net`-Domain) dürfen **niemals hardcoded** in YAML-Configs oder Markdown-Dokumentation vorkommen – sie würden im Public Repo landen.

**Konvention:**
- **In YAML-Dateien:** `!secret domain_extern_<name>` (ESPHome `secrets.yaml`)
- **In Markdown-/sonstigen Dateien:** `{domain_extern_<name>}` als Platzhalter
- **Auflösung:** Die echten Werte stehen lokal in `config/secrets.yaml` (gitignored)
- **Agenten:** Beim Lesen/Schreiben dieser Dateien die Platzhalter aus `secrets.yaml` ersetzen

**Aktuelle Einträge in `secrets.yaml`:**
- `domain_extern_music` – Music Assistant
- `domain_extern_dev` – Dev-Server (Assets/Bilder)
- `domain_extern_home` – Hauptdomain
- `domain_extern_code` – Gitea/Code
