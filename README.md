# Software-Based 3D Renderer

A project focused on building a software-based rendering engine on the CPU to explore the fundamentals of 3D graphics and understand the abstractions used to control the screen.

## Features

- **Software Rendering**: Implement 3D rendering entirely on the CPU without relying on GPU acceleration.
- **Learning-Oriented**: Designed to gain a deeper understanding of 3D graphics fundamentals, including transformations, shading, and rasterization.
- **Screen Control Abstractions**: Explore how low-level abstractions interact to manipulate the screen and render visuals.

## Getting Started

Follow these steps to set up and run the project on your local machine.

### Prerequisites

Ensure the following dependencies are installed:

- **CMake**: Build system for compiling the project.

### Installation

Step-by-step instructions to install and set up the project:

```bash
# Clone the repository
$ git clone https://github.com/yourusername/yourproject.git

# Navigate to the project directory
$ cd yourproject

# Create a build directory and navigate to it
$ mkdir build && cd build

# Configure the project with CMake
$ cmake ..

# Build the project
$ cmake --build .
```

> **Note:** The CMakeLists is tailored for MSVC but should work with other compilers as well.