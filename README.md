# Parallel Implementation of Conway's Game of Life

This repository contains the serial and parallel implementations of **Conway's Game of Life** using **OpenMP** and **Pthreads**. The project explores the performance differences between serial and parallel implementations, focusing on task-based and data-parallel approaches.

---

## Repository Structure

- **`life`**: Serial implementation of Conway's Game of Life.
- **`life_OpenMP`**: OpenMP-based parallel implementation (data-parallel).
- **`life_OpenMP_task`**: OpenMP-based task-parallel implementation.
- **`life_Pthreads`**: Pthreads-based parallel implementation (data-parallel).
- **`life_Pthreads_task`**: Pthreads-based task-parallel implementation.
- **`plot`**: Source code for plotting the simulation results.
- **`real_rand`**: Utility for generating random initial configurations.
- **`timer`**: Utility for measuring execution time.
- **`Makefile`**: Build script for compiling the project.
- **`CONWAY’S GAME OF LIFE, REPORT`**: Detailed report on the experimental analysis and findings.

---

## How to Build and Run

1. **Clone the repository**:
   ```bash
   git clone <repository-url>
   cd <repository-folder>
   ```

2. **Compile the project**:
   ```bash
   make
   ```

3. **Run the serial implementation**:
   ```bash
   ./life -n <grid-size> -i <iterations> -p <probability> -g <pattern>
   ```

4. **Run the OpenMP implementation**:
   ```bash
   ./life_OpenMP -n <grid-size> -i <iterations> -p <probability> -g <pattern>
   ```

5. **Run the Pthreads implementation**:
   ```bash
   ./life_Pthreads -n <grid-size> -i <iterations> -p <probability> -g <pattern>
   ```

---

## Parameters

- `-n <grid-size>`: Size of the grid (e.g., 500 for a 500x500 grid).
- `-i <iterations>`: Number of iterations to simulate.
- `-p <probability>`: Probability of a cell being alive initially.
- `-g <pattern>`: Initial pattern (1 for 2x2 Block, 2 for Gosper Glider Gun).

---

## Results

The performance of each implementation is analyzed in the report, including runtime comparisons and scalability insights. Key findings include:

- **Serial vs. Parallel**: Pthreads and OpenMP show modest speedups over the serial implementation.
- **Task Parallelism**: Task-based implementations (`life_OpenMP_task` and `life_Pthreads_task`) are less efficient due to overhead.
- **Grid Size Impact**: Larger grids benefit more from parallelization.

---

## Future Work

- Explore **hybrid parallelism** (e.g., combining OpenMP tasks with SIMD).
- Implement **GPU offloading** for further performance gains.
- Evaluate performance on **larger grids** and more complex patterns.

---

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

