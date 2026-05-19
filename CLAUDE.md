# CLAUDE.md

Behavioral guidelines to reduce common LLM coding mistakes. Merge with project-specific instructions as needed.

**Tradeoff:** These guidelines bias toward caution over speed. For trivial tasks, use judgment.

## 1. Think Before Coding

**Don't assume. Don't hide confusion. Surface tradeoffs.**

Before implementing:
- State your assumptions explicitly. If uncertain, ask.
- If multiple interpretations exist, present them - don't pick silently.
- If a simpler approach exists, say so. Push back when warranted.
- If something is unclear, stop. Name what's confusing. Ask.

## 2. Simplicity First

**Minimum code that solves the problem. Nothing speculative.**

- No features beyond what was asked.
- No abstractions for single-use code.
- No "flexibility" or "configurability" that wasn't requested.
- No error handling for impossible scenarios.
- If you write 200 lines and it could be 50, rewrite it.

Ask yourself: "Would a senior engineer say this is overcomplicated?" If yes, simplify.

## 3. Surgical Changes

**Touch only what you must. Clean up only your own mess.**

When editing existing code:
- Don't "improve" adjacent code, comments, or formatting.
- Don't refactor things that aren't broken.
- Match existing style, even if you'd do it differently.
- If you notice unrelated dead code, mention it - don't delete it.

When your changes create orphans:
- Remove imports/variables/functions that YOUR changes made unused.
- Don't remove pre-existing dead code unless asked.

The test: Every changed line should trace directly to the user's request.

## 4. Goal-Driven Execution

**Define success criteria. Loop until verified.**

Transform tasks into verifiable goals:
- "Add validation" → "Write tests for invalid inputs, then make them pass"
- "Fix the bug" → "Write a test that reproduces it, then make it pass"
- "Refactor X" → "Ensure tests pass before and after"

For multi-step tasks, state a brief plan:
```
1. [Step] → verify: [check]
2. [Step] → verify: [check]
3. [Step] → verify: [check]
```

Strong success criteria let you loop independently. Weak criteria ("make it work") require constant clarification.

---

**These guidelines are working if:** fewer unnecessary changes in diffs, fewer rewrites due to overcomplication, and clarifying questions come before implementation rather than after mistakes.

## Project
- Engine: UE 5.6.x
- Language: C++ + Blueprint
- Source: Source/SlayTheChampions/

## Code Navigation
- 심볼/클래스 위치 찾을 때: `grep "이름" tags` 먼저 확인
- 파일 추가/삭제 후 태그 갱신:
  `ctags -R --c++-kinds=+pf --fields=+iaSn --extras=+q --languages=C++ --exclude=Intermediate --exclude=Saved --exclude=Binaries --exclude=.git Source/SlayTheChampions/`

## Coding Rules
- 복잡한 로직 → C++
- 입력 처리, VFX 연결 → Blueprint
- UPROPERTY / UFUNCTION 매크로 필수
- 헤더엔 전방선언 우선, 꼭 필요할 때만 #include

## [Role & Instructions]
You are an expert developer. When modifying or refactoring code, you must strictly adhere to the following commenting and naming rules. These rules apply to both modifying existing code and writing new code.

## [Strict Rules]
1. Do NOT delete comments: Never remove, omit, or delete any existing comments during the refactoring or modification process. Keep them intact unless the underlying logic changes fundamentally.
2. Comments in Korean: All explanations, inline comments, and docstrings must be written EXCLUSIVELY in Korean (한국어).
3. Variables in English: All variable names, function names, class names, and identifiers must be written EXCLUSIVELY in English. Do not use Korean for any identifiers.

[Task]
(Here, describe the specific task you want to achieve. e.g., "Refactor the movement logic in PlayerController.cs")