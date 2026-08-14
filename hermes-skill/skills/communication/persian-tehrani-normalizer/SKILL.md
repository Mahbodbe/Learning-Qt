---
name: persian-tehrani-normalizer
description: Normalize Persian text output from LLMs to consistent Tehrani/colloquial register (مهریا tone)
category: communication
version: "1.0"
---

# Persian Tehrani Normalizer Skill

## Purpose
Normalize Persian text output from LLMs to consistent Tehrani/colloquial register (مهریا tone).

## Trigger
When LLM output in Persian needs normalization to consistent Tehrani register before user display.

## Rules

### Verb Normalization
| Formal | Tehrani |
|--------|---------|
| است | هست / ه |
| می‌باشد | هست / ه |
| دارد | داره |
| می‌دارد | داره |
| می‌روم | میرم |
| می‌خواهم | می‌خوام |

### Pronoun Normalization
| Formal | Tehrani |
|--------|---------|
| شما | تو |
| آیا | چی |
| جناب | تو / عزیزم |

### Vocabulary Replacements
| Formal | Tehrani |
|--------|---------|
| الان | الان |
| برای | برا / برا |
| است | هست / ه |
| نیست | نیس / نی |
| می‌باشد | هست |

### Implementation
```python
def normalize_tehrani(text: str) -> str:
    # Unicode normalization
    text = unicodedata.normalize('NFC', text)
    
    # Vocabulary replacements
    for formal, tehrani in VOCAB_MAP.items():
        text = text.replace(formal, tehrani)
    
    # Verb pattern replacements
    for pattern, replacement in VERB_PATTERNS:
        text = re.sub(pattern, replacement, text)
    
    return text
```

## Usage
Pipe LLM Persian output through this normalizer before user display.