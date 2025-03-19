# Mini Game Bot

This project plays the mini game inside Avataria. It is built using C++ and CMake.
![Build Status](https://github.com/quickwritereader/donraulava/actions/workflows/project.yml/badge.svg)

## Prerequisites

- Windows operating system
- Visual Studio Code
- CMake
- Microsoft Visual C++ (MSVC) compiler
- PowerShell

## Dependencies

- OpenCV 4.11

## Installation

1. **Install OpenCV 4.11:**
    - Follow the instructions on the [OpenCV installation page](https://opencv.org/releases/) to install OpenCV 4.11 on your system.

2. **Set up OpenCV environment variables:**
    - Add the OpenCV `bin` directory to your system's PATH environment variable. This allows the executable to find the OpenCV DLLs at runtime.
    ```sh
    setx -m PATH "C:\path\to\opencv\build\x64\vc15\bin;%PATH%"
    ```

3. **Configure CMake to find OpenCV:**
    - Ensure that CMake can find the OpenCV installation by setting the `OpenCV_DIR` environment variable to the location of the OpenCVConfig.cmake file.
    ```sh
    setx -m OpenCV_DIR "C:\path\to\opencv\build"
    ```

## Getting Started

1. **Clone the repository:**
    ```sh
    git clone https://github.com/yourusername/donRaulAva.git
    cd donRaulAva
    ```

2. **Configure the project with CMake Presets:**
    - Open PowerShell and run the following commands:
    ```sh
    cmake --preset x64-relese
    ```

3. **Build the project:**
    ```sh
    cmake --build --preset x64-relese
    ```

4. **Run the application:**
    ```sh
    .\out\build\x64-relese\donRaulAva.exe
    ```

## Using Visual Studio Code

1. **Open the project folder in Visual Studio Code:**
    ```sh
    code .
    ```

2. **Configure the CMake Tools extension:**
    - Install the CMake Tools extension from the Visual Studio Code marketplace.
    - Open the Command Palette (`Ctrl+Shift+P`) and select `CMake: Configure`.

3. **Build and run the project:**
    - Use the CMake Tools extension to build the project by selecting `CMake: Build`.
    - Run the executable from the terminal or use the debugging features of Visual Studio Code.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Acknowledgements

- Thanks to the developers of CMake and Visual Studio Code for their excellent tools.
- Gratitude to the CobraxAva team for their support and contributions.

## Donation  
  If you would like to support us, please consider making a donation to our Litecoin address:

  ### Litecoin Address:
  ```
  ltc1qj3l4mlh3wk6ld9348hk2rvfndfgnv2feqz9fqe
  ```
    
  ### Thank you for your support!
