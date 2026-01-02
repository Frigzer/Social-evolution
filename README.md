# Evolutionary Game Theory Simulator

> **Spatial simulation of evolutionary game theory dynamics (Iterated Prisoner's Dilemma and more).**

## Overview

This project simulates social evolution using **Game Theory** on a 2D grid. Agents interact with their neighbors playing games like the **Prisoner's Dilemma**, **Snowdrift**, or **Stag Hunt**. Based on their success (payoff), strategies evolve over time through biological reproduction (Death-Birth) or social imitation.

## Key Features

### Agent Strategies

* **Always Cooperate / Defect:** Baseline strategies.
* **Tit-For-Tat (Spatial):** Agents remember the last interaction with *each* specific neighbor (Pairwise Memory).
* **Pavlov (Win-Stay, Lose-Shift):** Universal implementation that adapts to any payoff matrix based on aspiration levels.
* **Discriminator:** Uses global reputation to decide whether to cooperate.

### Simulation Mechanics

* **Evolutionary Models:** Support for **Death-Birth** (Selection driven by fitness) and **Imitation** (Fermi rule / Best Neighbor).
* **Spatial Dynamics:** Agents can migrate based on local success (Success-driven migration).
* **Identity Persistence:** Unique Agent IDs ensure memory is reset correctly when neighbors change.
* **Customizable Games:** Real-time editing of Payoff Matrix (R, S, T, P).

## Tech Stack

* **Language:** C++20
* **Build System:** CMake
* **Graphics:** SFML 3.0.1
* **GUI:** ImGui + ImGui-SFML
* **Parallelism:** OpenMP

## Building & Installation

This project uses **CMake FetchContent**, so you don't need to manually install SFML or ImGui. Ensure you have a C++ compiler, CMake, and Git installed.

### Prerequisites

* **Windows:** Visual Studio or Visual Studio Code.
* **Linux:** GCC/Clang with OpenMP support (`libomp-dev`).

## Usage

1. **Left Panel:** Displays the simulation grid or real-time plots (History of population, Reputation).
2. **Right Panel (Controls):**
* **Start/Pause:** Control the simulation flow.
* **Strategies:** Toggle which agents are allowed to spawn/mutate.
* **Game Matrix:** Choose a preset (Prisoner's Dilemma, Stag Hunt) or manually tune R/S/T/P values.
* **Evolution Parameters:** Adjust Mutation Rate, Selection Strength (Beta), Fermi Noise (K).
 
