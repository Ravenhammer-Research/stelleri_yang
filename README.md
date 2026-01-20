# yang — build & test


Minimal instructions to build and run the project on Debian/Ubuntu-like systems.

Prerequisites

- A C++ toolchain and cmake: `build-essential`, `cmake`, `pkg-config`
- libyang dev headers/libraries (if you built/installed libyang locally, see notes below)
- ATF libraries and kyua for running tests: `libatf-dev`, `libatf-c++-dev`, `kyua`

Example apt (package names may vary by distro):

```bash
sudo apt update
sudo apt install -y build-essential cmake pkg-config \
  libyang-dev libatf-dev libatf-c++-dev kyua
```

If you installed `libyang` yourself (for example v4.2.2) into `/usr/local`, ensure the headers and libraries are available (the default `kDefaultSearchPaths` in `include/Yang.hpp` include the usual locations under `/usr/local/share/yang`):

- libyang library: `/usr/local/lib` (run `sudo ldconfig` if needed)
- libyang modules (shared models): `/usr/local/share/yang/modules/libyang/`

If you want the upstream YANG models available for testing, clone the YangModels repository into the standard modules path:

```bash
sudo git clone https://github.com/YangModels/yang /usr/local/share/yang/modules/yang
```

Build libyang from source (example)

If you need to build a specific libyang version (for example v4.2.2) and install it to `/usr/local`, use:

```bash
git clone https://github.com/CESNET/libyang.git
cd libyang
# checkout the desired tag, e.g. v4.2.2
git checkout v4.2.2
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j 4
sudo cmake --install .
sudo ldconfig
```

After installing libyang, clone the YangModels repository into the system modules path (requires root):

```bash
sudo git clone https://github.com/YangModels/yang /usr/local/share/yang/modules/yang
```

Build

```bash
# Configure (out-of-source)
cmake -S . -B build

# Build
cmake --build build -j 4
```

Run tests

Using kyua (recommended if installed):

```bash
kyua test --build-root=build -k build/Kyuafile
```

Run a single test binary directly:

```bash
./build/TestIetfInterfaces
```

Notes

- `kDefaultSearchPaths` in [include/Yang.hpp](include/Yang.hpp) already contains the common search paths used by this project, including `/usr/local/share/yang/modules/libyang/` and other standard locations. If your YANG files are installed elsewhere, either copy them into one of those locations or update `kDefaultSearchPaths` in the source.
- If you installed `libyang` to `/usr/local`, you may need to run `sudo ldconfig` so the runtime linker finds the library.

