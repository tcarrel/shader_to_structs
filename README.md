#GLSL To C

Utility used in the compilation process of the main program.

Processes all OpenGL shader language files in the current directory and
combines them into a single c-header file.  This is to simplify sending the
shaders to the GPU and compiling them on the gpu.

I created this to be used withing my senior project.

##Usage

### glsl_to_c [output filename]  
 
##Output Filename
The [output filename] is parsed and converted to all caps with special
characters changed to underscores (_) and suffixed and prefixed with
underscores (_) for the precompiler definition.  For example, *shader.h* will
become _SHADER_H_ and be affixed to the #define and #ifndef precompiler
directives in the header file.

##Input filenames
The filenames for OpenGL shader files is important as well.  In order to follow
a convention, of sorts, filenames for glsl code should be as follows:

&nbsp;[name].[type].glsl 


* [name] is the name of the shader this can be arbitrary, but should be a name
that makes sense within the rest of the code.  It should also be a name that
can be used as a valid variable name within c/c++.
* [type] is the type of the shader, i.e.: vertex, fragment, etc.  The types are
abreviated as follows:
* * v - vertex
* * f - fragment
* * others will be decided as development on the main program progresses.
* glsl is the extension and must not be changed as this is how the program
decides which files to include in its output.
* No periods other than those listed should be used in the filename.
* [name] and [type] together are used to generate the name of the instance of
shader code.  For example, if the filename is *shiny.v.glsl* then the name used
for the variable with in the header will be SHINY_v.
* Shaders can be given different names when they are later compiled on the gpu.

##Structure of generated data.

Shaders in the generated header are created as const structs with the following
declaration:  

```c
struct shader_code  
{  
  char* code;  
  int   size;  
}
```

