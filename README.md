# Grab Fare Calculator (C++17)

## Overview
This program simulates Grab’s fare system. It calculates ride fares based on:
- Vehicle type (Economy, Premium, Bike)
- Trip distance and duration
- Peak-hour surcharge
- Promo codes (GRAB10, STUDENT15, SUPER20)
- Minimum fare enforcement

It was developed as part of the **LDCW6123 – Fundamentals of Digital Competence for Programmers** project.

---

## Build & Run

### Local (if you have g++)
```bash
g++ -std=c++17 -Wall -Wextra -O2 grab_fare.cpp -o grab
./grab
