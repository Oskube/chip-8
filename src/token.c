#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "token.h"
#include "opcodes.h"

//#define DEBUG_PRINT( fmt, ... )  fprintf(stderr, "\t\t%s(...): " fmt, __FUNCTION__,__VA_ARGS__)
//#define DEBUG_PRINT( fmt, ... )

#define MAX_PROG_LEN 0x1000

typedef struct _node
{
    void* content;
    struct _node* next;
} node;

/* Structs used for output */
typedef struct
{
    const char* name;
    unsigned    name_len;

    unsigned    value;
} var;

typedef struct
{
    node* variables;

    unsigned char* codeBuffer; // Chip8 code compiled
    unsigned bufferPos; // Current position in buffer
    unsigned bufferLen;

    unsigned rows;
} compile_data;

void CompileDataFree(compile_data* c_data);
void DumpVariables(compile_data* c_data);
void DumpCompiledBytes(compile_data* c_data);


/**
 *  Tokenizer begin
 */
enum
{
    TOKEN_REGISTER = 1,
    TOKEN_STRING,
    TOKEN_NUMBER,
    TOKEN_MNEMONIC,
    TOKEN_LABEL,
    TOKEN_COMMA,
    TOKEN_ASSIGN,
    TOKEN_EOL,
    TOKEN_END
};

const char* token_to_str[] = {
    "",
    "TOKEN_REGISTER",
    "TOKEN_STRING",
    "TOKEN_NUMBER",
    "TOKEN_MNEMONIC",
    "TOKEN_LABEL",
    "TOKEN_COMMA",
    "TOKEN_ASSIGN",
    "TOKEN_EOL",
    "TOKEN_END"
};

typedef struct {
    unsigned type;
    unsigned len;
    union { // TODO: union could be utilized more and bunionr
        unsigned num;
        void* ptr;
    };
} token;

// Linked list functions
node* NodeAdd(node* root, void* content);
void NodeFree(node* root, void (*fptrReleaseContent)(void*));
void NodeIterate(node* root, void (*fptr)(node*));

// Token list functions
node* TokenAppend(node* root, unsigned type, void* ptr, unsigned len);
void TokenFree(void* t);
token* TokenGetFromNode(node* node);
token* TokenGetNext(node* node);
void PrintToken(node* n);
void PrintAllTokens(node* root);

bool isNumber(char c, bool hex);
bool isLetter(char c);
char peekNext(const char* c);
bool charInArray(char c, const char* array);
bool isMnemonic(const char* str, unsigned len);
bool isRegister(const char* str, unsigned len);

node* ParseToTokens(const char* src);
bool ProcessTokens(node* tokens, compile_data* data);


#ifdef TEST_TOKEN
// Used for quick testing
const char* source = \
    "Testi merkkijono\n"
    "Toinen rivi\n"
    "line = 0x1911  ;KOMMENTTI asd asda dadsa dasd wrqr q 132414 asczx c 34 0x123\n"
    "BeGIN:\n"
    "line = 0x1911  ;KOMMENTTI asd asda dadsa dasd wrqr q 132414 asczx c 34 0x123\n"
    "CLS\n"
    "MOV  V2, V4\n"
    "JMP  V0, 12,\n"
    "Label:MOV  I, 123\n"
    "MOV  I, line\n"
    "[I] ST DT I K F B V0 V1 V2 V3 V4 V5 V6 V7 V8 V9 V10 V11 V12 V13 V14 V15 V16 V 1\n";

int main()
{

    unsigned len = strlen(source);
    char* src_code = (char*)malloc(len + 1);
    memcpy( src_code, source, len );
    src_code[len] = '\0';

    node* tok = ParseToTokens( src_code );
    PrintAllTokens(tok);

    compile_data data = {0};
    ProcessTokens(tok, &data);
    DumpVariables( &data );
    DumpCompiledBytes( &data );

    CompileDataFree( &data );

    NodeFree(tok, TokenFree);
    free(src_code);

    return 0;
}
#endif

bool CompileSource(const char* src, unsigned char** compiled_output, unsigned* output_len)
{
    node* tok = ParseToTokens( src );

    compile_data data = {0};
    bool status = ProcessTokens(tok, &data);
    //DumpVariables( &data );
    //DumpCompiledBytes( &data );

    if (status)
    {
        *compiled_output = (unsigned char*)calloc(data.bufferLen, sizeof(unsigned char));
        memcpy(*compiled_output, data.codeBuffer, data.bufferLen);
    }

    *output_len = data.bufferPos;
    CompileDataFree( &data );
    NodeFree(tok, TokenFree);

    return status;
}

node* NodeAdd(node* root, void* content)
{
    node* new_node = (node*)malloc(sizeof(node));
    new_node->next = NULL;
    new_node->content = content;

    if (root)
    {
        node* cur = root;
        while (cur->next) cur = cur->next;
        cur->next = new_node;
    }
    else
    {
        root = new_node;
    }
    return root;
}

void NodeFree(node* root, void (*fptrReleaseContent)(void*))
{
    while(root)
    {
        node* n = root->next;
        fptrReleaseContent(root->content);
        free(root);
        root = n;
    }
}

void NodeIterate(node* root, void (*fptr)(node*))
{
    while(root)
    {
        node* n = root->next;
        fptr(root);
        root = n;
    }
}

node* TokenAppend(node* root, unsigned type, void* ptr, unsigned len)
{
    token* t = (token*)malloc(sizeof(token));
    t->type = type;
    t->ptr  = ptr;
    t->len  = len;
    //printf("Adding token!\n");

    return NodeAdd(root, (void*)t);
}

void TokenFree(void* t)
{
    token* tok = (token*)t;
    switch( tok->type )
    {
        case TOKEN_REGISTER:
        case TOKEN_STRING:
        case TOKEN_NUMBER:
        case TOKEN_MNEMONIC:
        case TOKEN_LABEL:
        case TOKEN_COMMA:
        case TOKEN_ASSIGN:
        case TOKEN_EOL:
        default: break;
    }
    free(tok);
}

token* TokenGetFromNode(node* node)
{
    if (!node) return NULL;
    return (token*)(node->content);
}
token* TokenGetNext(node* node)
{
    if (!node)
    {
        return NULL;
    }
    return TokenGetFromNode( node->next );
}

void PrintToken(node* n)
{
    token* t = TokenGetFromNode(n);

    printf("%p: %p:\t\t%s(%u)\t:: %p : %u", (void*)n, (void*)t, token_to_str[ t->type ], t->type, t->ptr, t->len);
    if (t->type == TOKEN_STRING   ||
        t->type == TOKEN_MNEMONIC ||
        t->type == TOKEN_REGISTER ||
        t->type == TOKEN_LABEL )
    {
            printf("\t\t'%.*s'", t->len, (char*)t->ptr);
    }

    printf("\n");
}

void PrintAllTokens(node* root)
{
    printf("Printing tokens::\n");
    NodeIterate(root, PrintToken);
    printf("=================\n");
}

bool isNumber(char c, bool hex)
{
    if (       (c >= '0' && c <= '9') ||
        (hex && c >= 'a' && c <= 'f') ||
        (hex && c >= 'A' && c <= 'F'))
    {
        return true;
    }
    return false;
}

bool isLetter(char c)
{
    if ((c >= 'a' && c <= 'z') ||
        (c >= 'A' && c <= 'Z'))
    {
        return true;
    }
    return false;
}

char peekNext(const char* c)
{
    return *(++c);
}

bool charInArray(char c, const char* array)
{
    for (const char* p = array; *p != '\0'; p++)
    {
        if (*p == c) return true;
    }
    return false;
}

bool isMnemonic(const char* str, unsigned len)
{
    bool found = false;
    char* tmp = (char*)malloc(len+1);
    strncpy(tmp, str, len);
    tmp[len] = '\0';

    for (unsigned i=0; mnemonic_list[i].fun != NULL; i++)
    {
        if (strcmp(tmp, mnemonic_list[i].mnemonic) == 0)
        {
            found = true;
            break;
        }
    }
    free(tmp);
    return found;
}

bool isRegister(const char* str, unsigned len)
{
    const char* regs[] = {
        "[I]",
        "ST",
        "DT",
        "I",
        "K",
        "F",
        "B",
        NULL
    };

    bool found = false;
    char* tmp = (char*)malloc(len+1);
    strncpy(tmp, str, len);
    tmp[len] = '\0';

    unsigned num = 0;
    if (sscanf(tmp, "V%d", &num) == 1 &&
        num >= 0 && num < 16)
    {
        found = true;
    }
    else
    {
        for (unsigned i=0; regs[i] != NULL; ++i)
        {
            if (strcmp(tmp, regs[i]) == 0)
            {
                found = true;
                break;
            }
        }
    }

    free(tmp);
    return found;
}

node* ParseToTokens(const char* src)
{
    if (!src) return NULL;

    node* t = NULL;
    const char* c = src;
    unsigned rows = 0;
    const char* row_begin = c;
    while( *c != '\0' )
    {
        bool tokenAdded = true;
        //printf("COMPARE '%c'\n", *c);
        switch(*c)
        {
            case ' ': // Ignore whitespace
            case '\t':
                break;
            case '=':
                t = TokenAppend(t, TOKEN_ASSIGN, NULL, 0);
                break;
            case ',':
                t = TokenAppend(t, TOKEN_COMMA, NULL, 0);
                break;
            case ';': // Comment, consume characters until EOL is reached
            {
                while (!charInArray(*c, "\n\0"))
                {
                    c++;
                }
            } // Fallthrough on purpose
            case '\n':
                t = TokenAppend(t, TOKEN_EOL, NULL, 0);
                rows++;
                row_begin = c + 1;
                break;
            default: 
                tokenAdded = false;
                break;
        }

        // Read integer number, dec or hex
        if (isNumber( *c, false ))
        {
            const char* begin = c;
            bool isHex = false;
            if (*c == '0' && peekNext(c) == 'x')
            {
                isHex = true;
                c++; // Because peekNext is used
            }

            char next = peekNext(c);
            while (isNumber(next, isHex))
            {
                next = peekNext(++c);
            }
            // Add number token
            unsigned num = strtol(begin, NULL, 0);
            //printf("Add number %.*s = %u\n", (c-begin)+1, begin, num);
            t = TokenAppend(t, TOKEN_NUMBER, (void*)num, 0);
            tokenAdded = true;
        }

        // Read string
        if (isLetter(*c))
        {
            const char* begin = c;
            char next = peekNext(c);
            while(isLetter(next) || isNumber(next, false))
            {
                next = peekNext(++c);
            }
            //printf("Add string '%.*s'\n", (c-begin)+1, begin);
            unsigned length = (c - begin) + 1;
            if (next == ':')
            {
                t = TokenAppend(t, TOKEN_LABEL, (void*)begin, length);
                c++; // Consume ':'
            }
            else if (isRegister(begin, length))
            {
                t = TokenAppend(t, TOKEN_REGISTER, (void*)begin, length);
            }
            else if (isMnemonic(begin, length))
            {
                t = TokenAppend(t, TOKEN_MNEMONIC, (void*)begin, length);
            }
            else
            {
                t = TokenAppend(t, TOKEN_STRING, (void*)begin, length);
            }
            tokenAdded = true;
        }

        // Handle case of "[I]"
        if (*c == '[')
        {
            if (peekNext(c) == 'I' &&
                peekNext(c+1) == ']')
            {
                t = TokenAppend(t, TOKEN_REGISTER, (void*)c, 3);
                c += 2;
                tokenAdded = true;
            }
        }

        if (!tokenAdded)
        {
            fprintf(stderr, "ERROR: %u:%u: Couldn't tokenize: '%c'\n", rows, (unsigned)(c - row_begin), *c);
        }
        c++;
    }
    return t;
}

/***
 *  Token parser
 */


void FreeVariable(void* var);
bool AddVariable(compile_data* c_data, const char* name, const unsigned name_len, unsigned val);
var* GetVariable(compile_data* c_data, const char* name, unsigned len);

void FreeVariable(void* var)
{
    free(var);
}

void CompileDataFree(compile_data* c_data)
{
    NodeFree(c_data->variables, FreeVariable);
    free(c_data->codeBuffer);
}

var* GetVariable(compile_data* c_data, const char* name, unsigned len)
{
    var* ret = NULL;

    //printf("%s: %.*s\n", __FUNCTION__, len, name );
    node* vars = c_data->variables;
    while( vars != NULL )
    {
        var* tmp = (var*)vars->content;
        const char* n = tmp->name;
        unsigned    l = tmp->name_len;
        if (l == len)
        {
            bool match = true;
            for(unsigned i = 0; i < len; ++i)
            {
                if (name[i] != n[i])
                {
                    match = false;
                    break;
                }
            }
            if(match)
            {
                ret = tmp;
            }
        }
        if (ret) break;
        vars = vars->next;
    }
    return ret;
}

bool AddVariable(compile_data* c_data, const char* name, const unsigned name_len, const unsigned val)
{
    // Check if variable already exists
    var* tmp = GetVariable(c_data, name, name_len);
    if (tmp)
    {
        //printf("%s: Variable '%.*s' already defined!\n", __FUNCTION__, name_len, name );
        return false;
    }

    // Create new variable
    var* new_label = (var*)malloc(sizeof(var));
    new_label->name     = name;
    new_label->name_len = name_len;
    new_label->value    = val;

    c_data->variables = NodeAdd( c_data->variables, (void*)new_label );
    //printf("%s: %.*s = %u\n", __FUNCTION__, new_label->name_len, new_label->name, new_label->value );

    return true;
}

void DumpVariables(compile_data* c_data)
{
    printf("Dump variables:\n");
    node* vars = c_data->variables;
    while( vars != NULL )
    {
        var* tmp = (var*)vars->content;
        const char* n     = tmp->name;
        unsigned    l     = tmp->name_len;
        unsigned    value = tmp->value;

        printf("Var:\t\t%.*s = %u\n", l, n, value);
        vars = vars->next;
    }
}

void DumpCompiledBytes(compile_data* c_data)
{
    printf("Dump compiled bytes:");
    unsigned char* pos = c_data->codeBuffer;
    for (unsigned i = 0; i < MAX_PROG_LEN; ++i, ++pos)
    {
        if (i % 16 == 0) printf("\n%#.4x\t", (unsigned short)i);
        printf("%.2x ", *pos);
    }
    printf("\n");
}

enum {
    STATE_NONE,
    STATE_START,
    STATE_ADD_LABEL,
    STATE_CREATE_VAR,
    STATE_ADD_MNEMONIC,
    STATE_END
};

void CreateAllLabels(compile_data* c_data, node* nodes);

typedef unsigned (*fStateHandler)(node** ptrCurNodes, compile_data* data);
unsigned StateStart      (node** nodes, compile_data* data);
unsigned StateAddLabel   (node** nodes, compile_data* data);
unsigned StateCreateVar  (node** nodes, compile_data* data);
unsigned StateAddMnemonic(node** nodes, compile_data* data);
fStateHandler SetState(unsigned next_state);

typedef struct
{
    unsigned token;
    unsigned dest;
} transition_map;

bool ProcessTokens(node* tokens, compile_data* data)
{
    /*
        State machine for token processing:

        START = { !TOKEN_LABEL!, TOKEN_MNEMONIC, TOKEN_STRING, TOKEN_EOL } => !ADD_LABEL!, ADD_MNEMONIC, CREATE_VAR, START
        !DONE IN SEPARATE PASS!  ADD_LABEL = { TOKEN_LABEL } => START
        CREATE_VAR = { TOKEN_STRING -> TOKEN_ASSIGN -> TOKEN_NUMBER -> EOL} => START
        ADD_MNEMONIC = { TOKEN_MNEMONIC -> ( TOKEN_NUMBER | TOKEN_STRING | TOKEN_REGISTER [-> TOKEN_COMMA] ) -> EOL } => START
    */
    bool status = true;

    data->codeBuffer = (unsigned char*)calloc(MAX_PROG_LEN, sizeof(unsigned char));
    data->bufferPos  = 0;//x200; // Program beginning
    data->bufferLen  = MAX_PROG_LEN;

    // First pass, find and create all labels
    CreateAllLabels(data, tokens);

    fStateHandler state = StateStart;
    node* cur = tokens;
    while(cur)
    {
        unsigned next_state = state(&cur, data);
        if (next_state == STATE_NONE)
        {
            next_state = STATE_START;
            cur = cur->next;
            printf("\t\tERROR: see row %u\n", data->rows);
            status = false;
        }
        state = SetState(next_state);
    }
    return status;
}

void CreateAllLabels(compile_data* c_data, node* nodes)
{
    unsigned old_pos = c_data->bufferPos;

    c_data->bufferPos = CHIP8_PROG_START;
    while(nodes)
    {
        token* tok = TokenGetFromNode(nodes);
        switch(tok->type)
        {
            case TOKEN_MNEMONIC: {
                c_data->bufferPos += 2;
            }
            default: {
                nodes = nodes->next;
            } break;
            case TOKEN_LABEL:
            {
                StateAddLabel(&nodes, c_data);
            } break;
        }
    }
    c_data->bufferPos = old_pos;
}

unsigned StateStart(node** nodes, compile_data* data)
{
    static const transition_map transitions[] = {
        { TOKEN_LABEL   , STATE_START        }, //STATE_ADD_LABEL    }, // labels added in separate pass
        { TOKEN_MNEMONIC, STATE_ADD_MNEMONIC },
        { TOKEN_STRING  , STATE_CREATE_VAR   },
        { TOKEN_EOL     , STATE_START        },
        { TOKEN_END     , STATE_END }
    };

    unsigned next_state = STATE_NONE;
    token* t = TokenGetFromNode(*nodes);
    unsigned type = t->type;

    for (unsigned i=0; transitions[i].dest != STATE_END; ++i)
    {
        if (type == transitions[i].token)
        {
            //printf("%s:: %u\n", __FUNCTION__, transitions[i].token);
            next_state = transitions[i].dest;
            break;
        }
    }
    if (next_state == STATE_START)
    {
        // Consume EOL token
        *nodes = (*nodes)->next;
        data->rows++;
    }

    return next_state;
}

unsigned StateAddLabel(node** nodes, compile_data* data)
{
    token* tok = TokenGetFromNode(*nodes);
    if (!AddVariable(data, (char*)tok->ptr, tok->len, data->bufferPos))
    {
        return STATE_NONE;
    }

    *nodes = (*nodes)->next;
    return STATE_START;
}

unsigned StateAddMnemonic(node** nodes, compile_data* data)
{
    bool success = true;
    node* n_mnemonic = *nodes;

    // Separate tokens to separate list
    bool possible_comma = false;
    unsigned o_count = 0;
    token* operands[3] = { NULL };
    while (true)
    {
        *nodes = (*nodes)->next;
        if (nodes == NULL) break;

        token* tok = TokenGetFromNode(*nodes);
        unsigned t_type = tok->type;

        if (t_type == TOKEN_NUMBER ||
            t_type == TOKEN_STRING ||
            t_type == TOKEN_REGISTER)
        {
            if ( o_count >= 3)
            {
                printf("ERROR: Mnemonic has over 3 operands\n");
                success = false;
                break;
            }

            if (t_type == TOKEN_STRING)
            {
                var* tmp = GetVariable(data, tok->ptr, tok->len);
                if (tmp == NULL)
                {
                    success = false;
                }
            }

            operands[o_count] = tok;
            o_count++;
            possible_comma = true;
        }
        else if (possible_comma && t_type == TOKEN_COMMA)
        {
            possible_comma = false;
        }
        else if (t_type == TOKEN_EOL)
        {
            if (o_count > 0 && !possible_comma)
            {
                printf("WARNING: Instruction ends with comma\n");
            }
            break;
        }
        else
        {
            printf("ERROR: Unknown token '%u'\n", t_type);
            success = false;
            break;
        }
    }

    if (!success) return STATE_NONE;

    // Find matching mnemonic and solve opcode
    token* t_mnemonic = TokenGetFromNode(n_mnemonic);
    unsigned m_count = GetMnemonicCount();
    for (unsigned i = 0; i < m_count; ++i)
    {
        const mnemonic* m = &(mnemonic_list[i]);
        if (strncasecmp((char*)(t_mnemonic->ptr), m->mnemonic, t_mnemonic->len) == 0 &&
            GetOperandCount(m) == o_count)
        {
            success = true;
            unsigned short opcode = m->base;
            for(unsigned j = 0; j < o_count; ++j)
            {
                token* t_oper = operands[j];
                const operand* oper = &(m->operands[j]);

                //printf("Token type: %i : %u/%u :: %u\n", i, j, o_count, t_oper->type);
                //printf("%s:: %s\n", m->mnemonic, oper->fmt);

                if (t_oper->type == TOKEN_NUMBER ||
                    t_oper->type == TOKEN_STRING)
                {
                    unsigned tok_value = 0;
                    if (t_oper->type == TOKEN_NUMBER)
                    {
                        tok_value = t_oper->num;
                    }
                    else /* TOKEN_STRING */
                    {
                        // Find matching variable if operand is string
                        var* tmp = GetVariable(data, t_oper->ptr, t_oper->len);
                        if (tmp == NULL)
                        {
                            success = false;
                        }
                        else
                        {
                            tok_value = tmp->value;
                        }
                    }

                    if (strcmp(oper->fmt, "%d") == 0)
                    {
                        opcode |= tok_value & oper->mask;
                        if ((tok_value & ~oper->mask) != 0)
                        {
                            printf("Warning: Value doesn't fit mask. (%x & %x = %x)\n", tok_value, oper->mask, tok_value & ~oper->mask);
                        }
                    }
                    else
                    {
                        success = false; // opcode not matching
                    }
                }
                else if (t_oper->type == TOKEN_REGISTER)
                {
                    // V registers, NOTE: there is special case of V0 used with JMP mnemonic!
                    if (*((char*)t_oper->ptr) == 'V' &&
                        (oper->mask == oper_0x00.mask || oper->mask == oper_00y0.mask))
                    {
                        char* start = (char*)t_oper->ptr + 1; // Ignore 'V'
                        unsigned num = strtol( start, NULL, 10 );
                        opcode |= ApplyMaskToValue( oper->mask, num );
                    }
                    else if(strncasecmp((char*)(t_oper->ptr), oper->fmt, t_oper->len) == 0)
                    {
                        /* do nothing, if registers matches */
                    }
                    else
                    {
                        success = false; // opcode not matching
                        break;
                    }
                }
                else
                {
                    /* ERROR: this shouldn't happen here */
                    success = false;
                }
            }
            if (success)
            {
                // Opcode built successfully
                unsigned pos = data->bufferPos;
                data->codeBuffer[ pos   ] = (opcode & 0xff00) >> 8;
                data->codeBuffer[ pos+1 ] =  opcode & 0x00ff;
                break;
            }
        }
    }

    data->bufferPos += 2;
    if (!success)
    {
        printf("ERROR: couldn't find matching operand for '%.*s' with %u operands.\n", t_mnemonic->len, (char*)t_mnemonic->ptr, o_count );
        return STATE_NONE;
    }

    return STATE_START;
}

unsigned StateCreateVar(node** nodes, compile_data* data)
{
    bool success = false;
    token* name = TokenGetFromNode(*nodes);
    *nodes = (*nodes)->next;

    token* value = TokenGetFromNode(*nodes);
    if (value && value->type == TOKEN_ASSIGN)
    {
        value = TokenGetNext(*nodes);
        if (value && value->type == TOKEN_NUMBER)
        {
            if (AddVariable(data, (char*)name->ptr, name->len, value->num))
            {
                *nodes = (*nodes)->next->next;
                success = true;
            }
        }
    }

    //printf("%s\n", __FUNCTION__ );
    if (!success)
    {
        return STATE_NONE;
    }
    return STATE_START;
}

fStateHandler SetState(unsigned next_state)
{
    fStateHandler handler = NULL;
    switch (next_state)
    {
        case STATE_START:
            return StateStart;
        case STATE_ADD_LABEL:
            return StateAddLabel;
        case STATE_ADD_MNEMONIC:
            return StateAddMnemonic;
        case STATE_CREATE_VAR:
            return StateCreateVar;
        case STATE_END:
        case STATE_NONE:
        default:
            break;
    }
    return handler;
}
