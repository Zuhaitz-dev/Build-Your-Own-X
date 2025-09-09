
// DISCLAIMER: The purpose of this project isn't making a functional
// copy of HTML, but providing a practical C example. Which means that
// many of the features will not be available; focus on the core logic.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Let's start with the data structures.

typedef enum
{
    TOKEN_TAG_OPEN,     // <body> or <h1>.
    TOKEN_TEXT,         // Whatever actual text to be honest.
    TOKEN_TAG_CLOSE,    // </body> or </h1>.
    TOKEN_EOF
} TokenType;

typedef struct
{
    TokenType   type;
    char*       value;  // The actual string content.
} Token;


// DOM Node (Document Object Model).
typedef struct Node
{
    char*           tag_name;
    char*           text_content;
    struct Node**   children;
    int             num_children;
} Node;

// Helper functions.

char* string_to_upper(const char* str)
{
    if (!str)
    {
        return NULL;
    }

    char* upper_str = strdup(str);
    for (int i = 0; upper_str[i]; i++)
    {
        upper_str[i] = toupper(upper_str[i]);
    }

    return upper_str;
}

void print_formatted_html(const char* html)
{
    int indent = 0;
    while (*html)
    {
        if ('<' == *html)
        {
            if ('/' == html[1])
            {
                indent -= 4;
                if (indent < 0) indent = 0;
            }
            
            if (html != 0)
            {
                printf("\n");
                for (int i = 0; i < indent; i++)
                {
                    printf(" ");
                }
            }
            
            if (html[1] != '/')
            {
                indent += 4;
            }
        }
        
        printf("%c", *html);
        html++;
    }
    printf("\n");
}

// Lexer.

Token* lexer(const char* html, int* num_tokens)
{
    int capacity = 10;  // This can be anything else, but for the example works.
    Token* tokens = malloc(sizeof(Token) * capacity);
    *num_tokens = 0; 
    size_t len = strlen(html);
    int i = 0;

    while(i < len)
    {
        if (isspace(html[i]))
        {
            i++;
            continue;
        }

        // Check for reallocation.
        if (*num_tokens >= capacity)
        {
            capacity *= 2;
            tokens = realloc(tokens, sizeof(Token) * capacity);
        }

        if ('<' == html[i])
        {
            int start = ++i;
            if ('/' == html[i]) // Closing tag.
            {
                start++;
                i++;
                while (html[i] != '>')
                {
                    i++;
                }

                int len = i - start;
                char* value = malloc(len + 1);
                strncpy(value, &html[start], len);
                value[len] =  '\0';
                tokens[*num_tokens].type  = TOKEN_TAG_CLOSE;
                tokens[*num_tokens].value = value;
            }
            else    // Opening tag.
            {
                while (html[i] != '>')
                {
                    i++;
                }

                int len = i - start;
                char* value = malloc(len + 1);
                strncpy(value, &html[start], len);
                value[len] = '\0';
                tokens[*num_tokens].type  = TOKEN_TAG_OPEN;
                tokens[*num_tokens].value = value;
            }
            i++;    // We move past the '>'.
        }
        else    // Text token.
        {
            int start = i;
            while (html[i] != '<' &&
                   html[i] != '\0')
            {
                i++;
            }
            
            int len = i - start;
            char* value = malloc(len + 1);
            strncpy(value, &html[start], len);
            value[len] = '\0';
            tokens[*num_tokens].type  = TOKEN_TEXT;
            tokens[*num_tokens].value = value;
        }
        (*num_tokens)++;
    }
    // EOF token.
    
    tokens[*num_tokens].type  = TOKEN_EOF;
    tokens[*num_tokens].value = NULL;
    (*num_tokens)++;

    return tokens;
}

// Parser.

// For our recursive parser we use a global token index.
int current_token_index = 0;

Node* parse(Token* tokens)
{
    Token current_token = tokens[current_token_index];

    if (current_token.type != TOKEN_TAG_OPEN)
    {
        fprintf(stderr, "[PARSE ERROR]: Expected an opening tag.\n");
        return NULL;
    }

    // Creating current node.
    Node* node = calloc(1, sizeof(Node));
    node->tag_name = strdup(current_token.value);
    current_token_index++;

    node->num_children = 0;
    node->children = NULL;

    while(tokens[current_token_index].type != TOKEN_TAG_CLOSE)
    {
        if (TOKEN_EOF == tokens[current_token_index].type)
        {
            fprintf(stderr, "[PARSE ERROR]: Unterminated tag '%s'\n", node->tag_name);
            return node;    // Partially parsed node.
        }
        
        if (TOKEN_TEXT == tokens[current_token_index].type)
        {
            node->text_content = strdup(tokens[current_token_index].value);
            current_token_index++;
        }
        else if (TOKEN_TAG_OPEN == tokens[current_token_index].type)
        {
            node->num_children++;
            node->children = realloc(node->children, sizeof(Node*) * node->num_children);
            node->children[node->num_children - 1] = parse(tokens);
        }
    }

    // Let's check if the closing tag matches the opening one.
    if (strcmp(node->tag_name, tokens[current_token_index].value) != 0)
    {
        fprintf(stderr, "[PARSE ERROR]: Mismatched tags. Expected </%s>, got </%s>\n",
                node->tag_name,
                tokens[current_token_index].value);
    }

    current_token_index++;
    return node;
}

// Renderer.

void render(Node* node)
{
    if (!node)
    {
        return;
    }

    // Let's apply some special formatting, here you can play.
    if (0 == strcmp(node->tag_name, "h1") &&
        node->text_content)
    {
        char* upper_text = string_to_upper(node->text_content);
        printf("** %s **\n\n", upper_text);
        free(upper_text);
    }
    else if (0 == strcmp(node->tag_name, "p") &&
             node->text_content)
    {
        printf("%s\n", node->text_content);
    }

    // We render all children recursively.
    for(int i = 0; i < node->num_children; i++)
    {
        render(node->children[i]);
    }
}

// Memory management.

void free_tokens(Token* tokens, int num_tokens)
{
    for(int i = 0; i < num_tokens; i++)
    {
        free(tokens[i].value);
    }
    free(tokens);
}

void free_dom_tree(Node* node)
{
    if (!node)
    {
        return;
    }

    free(node->tag_name);
    free(node->text_content);

    for (int i = 0; i < node->num_children; i++)
    {
        free_dom_tree(node->children[i]);
    }
    free(node->children);
    free(node);
}

int main(void)
{
    const char* html = "<body><h1>Our cute example</h1><p>Hello, C and Assembly Developers Community!</p></body>";

    printf("The input (HTML):");
    print_formatted_html(html);
    printf("\n");

    int num_tokens = 0;
    Token* tokens = lexer(html, &num_tokens);

    Node* dom_root = parse(tokens);

    printf("Rendered output:\n");
    render(dom_root);

    free_tokens(tokens, num_tokens);
    free_dom_tree(dom_root);

    return 0;
}
