#ifndef TOKEN_H
#define TOKEN_H

/**
 *  \brief  Compile given source code
 *  \param[in]  src  Source to be compiled
 *  \param[out] compiled_output  Pointer to compiled data
 *  \note compiled_output must be freed!
 */
bool CompileSource(const char* src, unsigned char** compiled_output, unsigned* output_len);

#endif //TOKEN_H
