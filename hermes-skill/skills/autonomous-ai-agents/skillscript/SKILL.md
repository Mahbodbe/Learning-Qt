---
name: skillscript
description: Author, compile, approve, and execute declarative AI agent skills using the Skillscript language and runtime.
category: autonomous-ai-agents
---

# Skillscript Language and Runtime Guide

Skillscript is a constrained, declarative, domain-specific language inspired by Makefiles. It structures agent reasoning into persistent, versioned, inspectable, and reusable automation scripts, avoiding redundant frontier-model token usage and hallucination.

## Core Conceptual Model

1. **Trigger** (Optional): Ambient triggers that fire the skill autonomously.
   - `cron`: Time-based scheduler (e.g., `cron: 0 7 * * MON-FRI`).
   - `event`: Public external signal matched 1:1 with a registered skill name (e.g., `event: deploy-finished`).
2. **Process**: Declarative dependency Directed Acyclic Graph (DAG). Named targets declare their dependencies using the `needs:` keyword. The runtime resolves the topological sort and executes them.
3. **Deliver**: Production of results.
   - **Body-text output template**: Declared implicitly (the skill body text itself with `${var}` substitutions).
   - **`emit(text="...")`**: Imperative routing for transcript/dynamic shapes.
   - Delivery channels are configured via the `# Output:` header (e.g., `agent: oncall`, `template: greeting`, `none`).

---

## Language Anatomy & Lexical Conventions

Every `.skill.md` file consists of three components:

1. **Header Metadata**: `# Key: value` lines declaring name, vars, outputs, triggers, and fallback behaviors.
2. **Targets**: Interconnected execution blocks.
3. **Default Goal**: The `default:` statement declaring the terminal target the runtime targets.

### Rules of Engagement

- **Indentation**: Spaces only (parse error on tabs or mixed indentation). Consistent block depth is strictly enforced.
- **Variables & Interpolation**: Use `${VAR}` for binding evaluation.
- **Filters**: `${VAR|trim|json|url|contains:"x"|shell}` are resolved sequentially.
- **No positional arguments**: All calls (runtime intrinsic or external MCP dispatch) are strictly all-kwargs.

---

## Action and Operations Surface

### 1. Mutation Statements
- `$set VAR = value`: Evaluates and binds a value.
- `$append VAR <value>`: Appends an element to a list or concatenates a string. Requires explicit initialization.

### 2. Runtime Intrinsic Functions
- `emit(text=...)`: Appends to delivery stream.
- `shell(command="...", unsafe=true) [-> BIND]`: Executes shell commands via a default-deny allowlist.
- `file_read(path=...)`, `file_write(path=..., content=...)`: Local file operations against a default-deny filesystem allowlist.

### 3. External MCP Tool Dispatches
Prefix with `$` to mark external or state-affecting tool actions:
- `$ <connector_name>.<tool_name> kwarg1=val1, ... [-> BINDING]` (e.g., `$ ticketing_search query="project:INFRA" -> SHOWSTOPPERS`).

---

## Common CLI Commands

```bash
skillfile init                           # Initialize a clean skill repository
skillfile dashboard                      # Start the DashboardServer (local web UI at 127.0.0.1:7878)
skillfile lint <skill>                   # Validate grammar and connectors
skillfile compile <skill>                # Output compiled prompt
skillfile approve <skill>                # Operator-authorization step
skillfile execute <skill>                # Run the approved skill
skillfile fires <skill>                  # List trace logs
```

---

## Canonical Multi-Target Example

```markdown
# Skill: morning-brief
# Status: Draft
# Description: Summarize calendar, mailbox, and overnight notes.
# Vars: AGENT, BRIEF_HORIZON_HOURS=24
# Triggers: cron: 0 7 * * *
# Output: agent: ${AGENT}

calendar:
    $ calendar.list_events horizon_hours=${BRIEF_HORIZON_HOURS} -> EVENTS (fallback: "(calendar unavailable)")

mailbox:
    $ data_read mode=fts query="messages for ${AGENT}" limit=10 -> MAIL (fallback: "(mailbox unavailable)")

compose: needs: calendar, mailbox
    $ llm prompt="Compose morning brief. Events: ${EVENTS|json}. Mail: ${MAIL|json}." -> BRIEF (fallback: "Failed compose.")

default: compose
```
