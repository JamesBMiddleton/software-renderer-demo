# Software Renderer Demo

A software renderer and voxel mesh generator with no dependencies beyond a C compiler, hand-written from scratch.

<video src="https://github.com/user-attachments/assets/0eacf5be-3e56-4530-8acb-09343904379b" autoplay loop muted playsinline></video> 

This is an extract from a larger ongoing project to write a completely freestanding 3D game (no dependencies including libc!) that is runnable on the widest possible range of past, present and future hardware targettable by a C compiler.

For this demo, Sokol (a cross-platform I/O wrapper) is being used to pipe the rendered frames to the screen.
