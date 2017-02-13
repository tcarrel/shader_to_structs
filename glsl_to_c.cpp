/**
 *
 * \brief  Copies contents of glsl files into c-code files to be hard-coded and
 * included in the final program as c-style strings.
 *   The only purpose of this program is to allow shaders to be edited with
 * syntax highlighting in vim (or whichever program one chooses.)
 *
 *   "Modifies" the compilation process.
 *
 *   \file glsl_to_c.cpp
 *   \author Thomas R. Carrel
 */




#include<iostream>
using std::string;
using std::cout;
using std::cerr;
using std::endl;

#include<fstream>
using std::ifstream;
using std::ofstream;
using std::ostream;

#include<vector>
using std::vector;

//Debug
#include<cassert>
//using std::assert;

//Add options for different os's.
#include<dirent.h>

const char DQ = 0x22; ///< Store a double quote in a constant.


/**
 * The name of the generated type.   This macro is duplicated in the
 * generated source file so it can easily be used in the program.
 */
#define SHADER_TYPE_NAME "_shader_code"




/**
 * Inserts the comment at the top of the generated file.
 * \param out Reference to the output stream.
 * \param exec_name Name of the executable.
 * \param filename Name of the output file.
 */
void insert_comment( ostream& out, char* exec_name, string& filename )
{
    out <<
        "/**\n" <<
        " * \\file " << filename << "\n" <<
        " * \\author " << exec_name << "\n"
        " *\n" <<
        " *   Auto-generated header file containing code from all shaders" <<
        "used in this\n" <<
        " * program.  A list of the files used to generated this file can " <<
        "be found at\n" <<
        " * the bottom of this file.\n" <<
        " *\n" <<
        " * file generated by:     " << exec_name << endl <<
        " *\n" <<
        " */\n" << endl << endl << endl;
}


/**
 * Inserts the necessary info for the beginning of the header file.
 * \param out Reference to the output stream.
 * \param def_tag The identifier for the pre-compiler tags.
 */
void header_def_start( ostream& out, const char* def_tag )
{
    out << "#ifndef  SHADER_TYPE_NAME\n"
        << "# define SHADER_TYPE_NAME " << SHADER_TYPE_NAME 
        << " ///< A macro is used for the typename\n"
        << string(39, ' ') << "///< since it is automatically\n"
        << string(39, ' ') << "///< generated by another program.\n#endif\n\n"
        << "#ifndef  " << def_tag << "\n"
        << "# define " << def_tag << "\n" << endl;

    out << "#include<GL/glew.h>\n"
        << "#include<SDL2/SDL.h>\n"
        << "#include<SDL2/SDL_opengl.h>\n"
        << "\n"
        << "#include<GL/glu.h>\n"
        << "#include<GL/freeglut.h>\n" << endl;

    out << "/** Container for shader code.\n"
        << " *  Streamlines use of hard-coded shaders in OpenGL by allowing "
        << "them to be\n"
        << " *  in their own files with the use of syntactic highlighting.\n"
        << " *\n"
        << " */" << endl;

    out << "struct " << SHADER_TYPE_NAME << "\n"
        << "{\n"
        << "  GLchar* code; ///< Source text.\n"
        << "  GLuint  size; ///< Number of characters in the source text.\n"
        << "  const GLuint  id; ///< unique ID for each bit of shader code.\n"
        << "\n"
        << "/** Ctor.  Necessary because structs are stored as constants.\n"
        << " *\n"
        << " * param c C-string of the shader source code.\n"
        << " * param s The number of characters in the shader source.\n"
        << " */\n"
        << "  " << SHADER_TYPE_NAME << "( GLchar* c, GLuint s, GLuint i ) :\n"
        << "    code(c), size(s), id(i)\n"
        << "  {}\n"
        << "\n"
        /*
        << "  ~" << SHADER_TYPE_NAME << "( void )\n"
        << "   {\n"
        << "      if( code )\n"
        << "      {\n"
        << "        delete code;\n"
        << "        code = NULL;\n"
        << "      }\n"
        << "   }\n"
        << "\n"
        */
        << "};\n" << endl;
}



/** Closes the preprocessor directive.
 *
 *  param out Output stream for the file being written.
 *  param def_tag The name of the preprocessor tag.
 */
void header_def_end( ostream& out, const char* def_tag )
{
    out << "\n#endif /* " << def_tag << " */\n" << endl;
}


/** List all the files used in the code generation in a block comment.
 *
 *  param out Output stream for the file being written.
 *  param names A vector containing the names of all the files that were used.
 */
void file_listing( ostream& out, vector<string*> names )
{
    out <<
        "//\n" <<
        "// Summary of all files used for generation of this header:\n" <<
        "//" << endl;

    for( auto file = names.begin(); file != names.end(); ++file )
    {
        out << "// " << **file << endl;
    }

    out << "//" << endl;
}







int main( int argc, char* argv[] )
{
    if( argc < 2 )
    {
        cerr
            << "Missing output filename.\n"
            << "Usage:\n"
            << "  " << argv[0] << "[output filename]" << endl;
        return 1;
    }

    string of_name = argv[1]; ///< Output filename, taken from a commandline
                              ///< argument.

    string macro_name = "_";  ///< The preprocessor directive name is 
                              ///< generated here, an underscore is prepended.

    for( unsigned i = 0; i < of_name.length(); i++ )
    {
        if( 'a' <= of_name[i] && of_name[i] <= 'z' )
        {
            macro_name += of_name[i] - 0x20;
        }
        else if( 'A' <= of_name[i] && of_name[i] <= 'Z' )
        {
            macro_name += of_name[i];
        }
        else if( '0' <= of_name[i] && of_name[i] <= '9' )
        {
            macro_name += of_name[i];
        }
        else
        {
            macro_name += "_";
        }
    }
    macro_name += "_";


    DIR* directory = opendir( "./" ); ///< The current working directory's
                                      ///< File __.
    struct dirent* dir_dat; ///< Directory data.

    string filename; ///< Current file's name.
    ifstream inf;    ///< Input file stream.

    ofstream of; ///< The output file stream.
    of.open( of_name.c_str() );

    int id = 1;

    string c_name = of_name;
    size_t dot = c_name.find_last_of( '.' );
    if( dot == std::string::npos )
    {
        cerr << "Invalid file name.\n" << endl;
        return 0;
    }
    c_name.replace( dot, c_name.length() - dot, ".cpp");


    ofstream cfile;
    cfile.open( c_name.c_str() );
    

    bool commented = false, ///< Flag for the generated file's opening comment.
         begun = false; ///< Flag for the initial preprocessor directives in
    ///< the file.

    vector<string*> filenames; ///< Stores the names of all of the files used.
    vector<string*> inst_names;

    while( (dir_dat = readdir( directory )) )
    {
        filename = dir_dat->d_name;
        if( filename[0] != '.' ) // Ignore hidden files
        {
            //get file extension.
            /** Location of the period that begins the file extension.
            */
            size_t extension_start = filename.find_last_of( '.' );
            size_t type_start = filename.find_first_of( '.' ); ///< Location of
            ///< the first
            ///< period in
            ///< the
            ///< filename.
            if( extension_start != std::string::npos )
            {
                string extension = filename.substr( extension_start );
                if( extension == ".glsl" )
                {
                    filenames.push_back( new string(filename) );

                    /** The name of the current shader, taken from the
                     *  filename.
                     */
                    string shader_name = filename.substr( 0, type_start ); 
                    /** The type of the shader (vertex, fragment, etc).
                    */
                    string shader_type = filename.substr(
                            type_start + 1,
                            extension_start - (type_start + 1));
                    if( !commented )
                    {
                        insert_comment( of, argv[0], of_name );
                        insert_comment( cfile, argv[0], c_name );
                        commented = true;
                    }


                    if( !begun )
                    {
                        header_def_start( of, macro_name.c_str() );

                        cfile
                            << "\n#include " << DQ << of_name << DQ << "\n"
                            << endl;
                        begun = true;
                        header_def_end( of, macro_name.c_str() );
                    }

                    for( int i = 0; shader_name[i] != 0; i++ )
                    {
                        if( shader_name[i] > 0x40 && shader_name[i] < 0x7B )
                            shader_name[i] -= 0x20;
                    }

                    inf.open( filename.c_str() );

                    if( !inf.good() )
                    {
                        cerr
                            << "Could not read file <" << filename 
                            << ">, skipping." << endl;
                        inf.close();
                        continue;
                    }

                    /** Name of the instance of the shader code struct that
                     *  will be used to store the current shader.
                     */
                    string shader_var_name = shader_name + "_" + shader_type;


                    cfile << "/** From file:  " << filename << "\n */" << endl;

                    string file_text = ""; ///< The code from the current
                    ///< shader file as it will be
                    ///< output into the generated file.

                    string line; ///< The text from the currently processing
                    ///< line from the glsl-file.
                    string length; ///< A copy of the file_text string, but
                    ///< without the added quotes and such used to
                    ///< make it into a c-string that is also
                    ///< in a human-readable format and
                    ///< somewhat follows coding practices, but
                    ///< which would make its length incorrect.
                    for( 
                            getline( inf, line );
                            !inf.eof();
                            getline( inf, line ) )
                    {
                        if( line != "" )
                        {
                            // Nested for readability.
                            // Strips whole-line c-style comments,
                            // (Those beginning with // as the first two
                            // characters on the line.
                            if( line.substr( 0, 2 ) != "//" )
                            {
                                file_text += DQ + line + "\\n" + DQ + "\n";
                                length += line + "\n";
                            }
                        }
                    }

                    inst_names.push_back( new string( shader_var_name ) );
                    cfile
                        << SHADER_TYPE_NAME << " "
                        << shader_var_name << "(\n  ";
                    for( unsigned i = 0; i < file_text.length(); i++ )
                    {
                        cfile << file_text[i];
                        if( file_text[i] == '\n' )
                            cfile << "  ";
                    }
                    cfile
                        << ",\n  " << length.length() << ",\n  "
                        << id++ << endl;
                    cfile << ");\n\n" << endl;

                    inf.close();
                }
            }
        }
    }

    if( commented || begun )
    {
        file_listing( of, filenames );
        file_listing( cfile, filenames );
        
        of << "\n" << endl;
        cfile << "\n" << endl;


        of.close();
        of.open( "shader_externs.h" );
        of
            << "/** Include at the top of any .cpp files needing access to the "
            << "uncompiled\n"
            << " * shaders.  This isn't the best idea, but it's convenient.  "
            << "I'll remove this\n"
            << " * and do just do it manually later should it become a "
            << "problem.\n"
            << " */\n" << endl;

        for( unsigned i = 0; i < inst_names.size(); i++ )
        {
            of << "extern SHADER_TYPE_NAME " << *(inst_names[i]) << ";\n";
        }
        of << endl;

        filenames.clear();
        inst_names.clear();
    }
    else
    {
        cerr << "No files to process.\n" << endl;
        of << " ";
        cfile << " ";
    }
    cfile.close();
    of.close();
    return 0;
}
