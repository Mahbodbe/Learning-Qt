---
name: persian-tehrani-normalizer
description: Normalize Persian text to Tehrani colloquial dialect. Converts formal/regional Persian to Tehrani colloquial dialect.
category: creative
tags: [persian, farsi, tehran, colloquial, normalization, nlp]
version: 1.0.0
author: Hermes
created: 2026-07-28
---

# Persian Tehrani Normalizer

Normalizes Persian text to Tehrani colloquial dialect by applying linguistic rules for verb conjugation, pronoun normalization, vocabulary replacement, and colloquial patterns.

## Features
- Verb conjugation normalization (است → هست, دارد → داره)
- Pronoun normalization (شما → تو, کتاب شما → کتابت)
- Colloquial pattern application
- Tehrani vocabulary substitution

## Usage
```python
from persian_tehrani_normalizer import normalize_tehrani
text = "شما کتاب خود را دارید"
result = normalize_tehrani(text)
# Output: "تو کتاب خود را دارین"
```

## Rules
Vocabulary map and verb patterns are stored in `rules/` directory:
- `rules/vocab_map.json` — formal → tehrani word mappings
- `rules/verb_patterns.json` — regex verb conjugation patterns

## Testing
```bash
python3 /root/.hermes/skills/persian-tehrani-normalizer/test_normalizer.py
```
