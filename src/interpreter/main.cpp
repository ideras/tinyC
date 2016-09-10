#include <cstdio>
#include "ast.h"
#include "tokens.h"

DeclarationList *decl_list;
Statement *input;
extern map<string, VValue> vars;

int main()
{
    input = NULL;
    yyparse();
    
    if (decl_list != NULL) {
        VValue v;
        DeclarationList::iterator it = decl_list->begin();
        
        while (it != decl_list->end()) {
            Declaration *decl = *it;
            StringList::iterator it2 = decl->idList.begin();
            
            while (it2 != decl->idList.end()) {
                string id = *it2;
                
                v.type = decl->type;
                vars[id] = v;
                
                it2++;
            }
            it ++;
	    
	    delete decl;
        }
        
        delete decl_list;
    }

    if (input != 0) {
        input->execute();
    }
}
