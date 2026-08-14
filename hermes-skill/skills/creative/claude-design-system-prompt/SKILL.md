---
name: claude-design-system-prompt
description: "Design system prompt and skill library — opinionated, accessibility-aware, AI-slop-resistant design collaborator. Based on Trystan-SA/claude-design-system-prompt."
version: 1.0.0
author: Hermes Agent (adapted from Trystan-SA)
license: MIT
platforms: [linux, macos, windows]
metadata:
  hermes:
    tags: [design-system, UX, accessibility, prototyping, frontend]
    related_skills: [sketch, architecture-diagram, popular-web-designs]
---

# Claude Design System Prompt — Adapted

Use this skill when the user wants a **design-system-driven approach** — not one-off pages but a coherent visual language with components, tokens, and reviews.

Load this before any design/UI task when the user references "design system", "handmade style", "clean design", or "professional frontend".

---

## Core Principles

1. **Content discipline** — every element earns its place. No filler.
2. **Aesthetic discipline** — reject AI tropes (aggressive gradients, emoji decoration, rounded-corner cards, Inter-everywhere). Commit to a palette and tone.
3. **Visual hierarchy & rhythm** — size, color, weight, position, density, spacing scales.
4. **Accessibility** — WCAG AA+, semantic HTML, keyboard navigation, focus rings, reduced-motion.
5. **Interaction states** — hover, active, disabled, focus, loading, validation.
6. **System thinking** — components and tokens over one-off pages.
7. **Respect the medium** — real CSS Grid, `oklch()`, `text-wrap: pretty`, real interactive prototypes.
8. **Quality over quantity** — depth over breadth, polish every detail.

---

## Available Skills (Invokable Procedures)

### Production — Build Something
- **discovery-questions**: Kickoff question protocol — ask the right questions before designing.
- **frontend-aesthetic-direction**: Commit to a look when no brand exists. Generates 4+ directional options.
- **wireframe**: Low-fi exploration, 3+ layout variations.
- **make-a-deck**: Slide presentations in HTML.
- **make-a-prototype**: Interactive clickable prototype with real CSS and states.
- **make-tweakable**: Floating tweak panel for real-time adjustments.
- **generate-variations**: 3+ hi-fi variations across axes (color, layout, typography).

### System — Extract Structure
- **design-system-extract**: Pull tokens (colors, spacing, typography) from an existing source (site URL, screenshot, design file).
- **component-extract**: Inventory reusable components from a page or mockup.

### Review — Audit and Fix
- **accessibility-audit**: WCAG, semantic HTML, keyboard, motion preferences.
- **ai-slop-check**: Detect gradient abuse, emoji decoration, generic templates, house-style tropes.
- **hierarchy-rhythm-review**: Size/weight/color spacing scale audit.
- **interaction-states-pass**: Check all interactive states.
- **polish-pass**: Final umbrella review before delivery.

---

## Typical Workflows

### Greenfield (no brand)
```
discovery-questions → frontend-aesthetic-direction → wireframe → make-a-prototype → polish-pass
```

### Brand-existing
```
design-system-extract → generate-variations → make-tweakable → polish-pass
```

### Audit-existing
```
ai-slop-check → accessibility-audit → hierarchy-rhythm-review → interaction-states-pass → polish-pass
```

---

## When to Use Which Skill

| If user says... | Invoke skill |
|----------------|-------------|
| "I want a design for my shop" | `frontend-aesthetic-direction` first |
| "Check my page for issues" | `ai-slop-check` + `accessibility-audit` |
| "I have a reference site" | `design-system-extract` from that URL |
| "Make it interactive" | `make-a-prototype` |
| "Generate variations" | `generate-variations` |
| "Polish it" | `polish-pass` |

---

## Example Trigger

```markdown
**User:** "I want my resin shop to look clean and professional, like handmade Shopify themes."

**Agent:** Loads this skill → invokes `frontend-aesthetic-direction` → generates 4 visual directions → user picks one → invokes `make-a-prototype` → builds with tokens → `polish-pass`
```

---

## Implementation Patterns (from real Django shop)

See `references/django-shop-implementation-patterns.md` for production-grade patterns:
- Wishlist AJAX toggle (`@csrf_exempt`)
- Auth-aware nav (admin vs customer)
- Guarded send-to-bot (anonymous → login redirect)
- Static page pattern
- SiteSetting (global texts) + Page (full HTML pages) CMS
- TinyMCE CDN editor
- Product auto-code (filter by type)
- MEDIA_ROOT alignment with Nginx
- Login loop fix (HTTP)
- Low-RAM cron fallback over Celery
- Unfold RTL/LTR split via LANGUAGE_CODE
