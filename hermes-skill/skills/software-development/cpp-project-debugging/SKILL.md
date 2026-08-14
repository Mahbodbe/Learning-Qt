---
name: cpp-project-debugging
description: Systematic techniques for debugging C++ projects, including inheritance pitfalls, const correctness, linker errors, and constructor mismatch issues in multi-class ecosystem simulations.
version: 1.0.0
author: Hermes Agent
tags: [cpp, debugging, compiler-errors, inheritance, polymorphism, exercise, linker-errors, const-correctness, oop, stl]
---

# C++ Project Debugging Skill

## Overview
Covers the most common compilation and runtime pitfalls in multi-class C++ projects, especially those involving inheritance hierarchies, smart pointers, and STL containers. Based on real debugging sessions with student projects like ecosystem simulators.

## Inheritance & Base Class Design (Abstract Base Class)

**All abstract base classes MUST:**
```cpp
class Organism {
protected:  // Protected constructor — cannot instantiate abstract class directly
    Organism(int x, int y, int energy, int id) : x_(x), y_(y), energy_(energy), id_(id) {}
    
public:
    virtual ~Organism() = default;  // Virtual destructor for proper cleanup
    
    // Pure virtual functions
    virtual void act(World& world) = 0;       // Pure virtual (abstract)
    virtual void reproduce(World& world) = 0; // Pure virtual
    virtual char getSymbol() const = 0;
};
```

**Common mistakes:**
| Mistake | Impact | Fix |
|---------|--------|-----|
| Public constructor in ABC | Can accidentally instantiate `Organism` | Change to `protected` |
| Non-virtual destructor | Undefined behavior on polymorphic delete | Add `virtual ~Organism() = default;` |
| Missing `= 0` on any virtual method | Base class is not actually abstract | Make at least one method pure virtual |
| Defining `reproduce()` in base class AND making it pure virtual | Linker error (duplicate symbol) | Remove the definition; keep only the declaration (`= 0`) |

## Constructor Chain & Initializer Lists in Derived Classes

**Ensure the derived class constructor passes ALL required parameters to the base class in the correct order.**

```cpp
// Organism Base
Organism(int x, int y, int energy, int id, int reproduceThreshold, int moveCost, int foodEnergy = 0);

// Derived: Herbivore
Herbivore::Herbivore(int x, int y, int energy, int id,
                     int reproduceThreshold, int foodEnergy, int moveCost)
    : Organism(x, y, energy, id, reproduceThreshold, moveCost, foodEnergy)  // WRONG ORDER!
```

**The parameter order in the derived constructor must match the base class:**
- **Base expects:** `x, y, energy, id, reproduceThreshold, moveCost, foodEnergy`
- **Derived passed (wrong):** `x, y, energy, id, reproduceThreshold, foodEnergy, moveCost` (mismatch of parameter 5 & 6)

**Pro tip:** If the compiler produces `no matching constructor`, check the parameter order in the initializer list first.

## Include File Case Sensitivity (Linux)

**Linux filesystem is case-sensitive.** `#include "World.h"` and `#include "world.h"` refer to different files.

**A common cause of cryptic linker errors:** If all `.cpp` files have been patched to use `"World.h"` but the file on disk is `"world.h"`, compile will fail.

**Fix one-by-one with sed:**
```bash
# Normalize ALL includes to the actual file names on disk
sed -i 's/#include "World.h"/#include "world.h"/g' *.cpp *.h
```

## Const Correctness

**Getters that do NOT modify the object MUST be declared `const`:**
```cpp
class Organism {
    int getX() const;          // ✅ Read-only
    int getEnergy() const;     // ✅ Read-only
    bool isDead() const;       // ✅ Read-only
};
```

**Parameters that are only read (not modified) should be passed as `const&`:**
```cpp
std::pair<int, int> getSpawnPosition(const std::shared_ptr<Organism>& parent) const;
```

## Mutable Random Number Generator

When using `std::mt19937 rng` in a class that also has `const` methods, the RNG needs to be `mutable`:

```cpp
class World {
private:
    mutable std::mt19937 rng_;  // Can be mutated from const methods
};
```

**Without `mutable`:** Compiler error: `cannot modify member object 'rng_' in const member function`.

## Linker Errors: "Undefined reference"

**Cause:** A function declared in the `.h` file but never defined in the `.cpp` file.

**Fix:** Either:
1. Add the definition in `.cpp` (if the function has meaningful body)
2. If the function only has `= 0;` or `= default;`, ensure it's NOT also defined in `.cpp`:
   ```cpp
   // IN .h:
   virtual ~Organism() = default;
   
   // WRONG in .cpp:
   // Organism::~Organism() {}   ← Duplicate definition!
   
   // CORRECT: Remove the definition from .cpp entirely. The = default; in the header is enough.
   ```

## Re-definition of Constructors (Compile Error)

**Problem:** Constructor declared in the `.h` file (with inline definition) AND defined again in the `.cpp` file causes `redefinition of constructor` error.

**Solution:** Keep the constructor ONLY in the `.h` file (with inline or `= default;`), and remove the definition from the `.cpp` file entirely.

## Common STL Pitfalls

| Pitfall | Symptom | Fix |
|---------|---------|-----|
| `std::shuffle` requires `<algorithm>` + `<random>` + RNG | Compile error | Include both headers; provide seeded `std::mt19937` |
| `std::uniform_int_distribution<size_t>` needs `#include <random>` | Compile error | Add explicit include |
| `const std::vector<T>&` parameter binding to non-`const` rvalue | Compile error | Ensure function signature uses `const&` for read-only parameters |
| `mutable std::mt19937` not declared in class | Compile error in const method | Add `mutable` keyword to member variable |

## Compilation Command

Always compile with full warnings and C++17:
```bash
g++ -std=c++17 -Wall -Wextra -O2 *.cpp -o output
```

For debugging logic errors:
```bash
g++ -std=c++17 -g -O0 *.cpp -o output_debug
```
