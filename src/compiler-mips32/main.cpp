#include <cstdio>
#include <iostream>
#include "ast.h"
#include "tokens.h"

DeclarationList *decl_list;
Statement *input;
extern map<string, DataType> vars;
extern map<string, string> float_constants;

int yyparse();

int main()
{
    input = NULL;
    yyparse();
    
    if (decl_list != NULL) {
        DeclarationList::iterator it = decl_list->begin();
        
        while (it != decl_list->end()) {
            Declaration *decl = *it;
            StringList::iterator it2 = decl->idList.begin();
            
            while (it2 != decl->idList.end()) {
                string id = *it2;
                
                vars[id] = decl->type;
                
                it2++;
            }
            it ++;
	    
	    delete decl;
        }
        
        delete decl_list;
    }

    if (input != 0) {
        string code = input->generateCode();
        map<string, DataType>::iterator it = vars.begin();

        cout << ".data" << endl;
        cout << "  _endl: .asciiz \"\\n\"" << endl;

        while (it != vars.end()) {
                string id = it->first;
                if (it->second == DT_Int) {
                    cout << "  " << id << ": .word 0" << endl;
                } else {
                    cout << "  " << id << ": .double 0.0" << endl;
                }
                it++;
        }
        
        // Registering float literals
        map<string, string>::iterator it2 = float_constants.begin();
        
        while (it2 != float_constants.end()) {
            string cvalue = it2->first;
            string label = it2->second;
        
            cout << "  " << label << ": .double " << cvalue << endl;
            it2++;
        }

        cout << endl;
        cout << ".text" << endl;
        
        cout << code;
        
        cout << "# Exit\n"
             << "li $v0, 10\n"
             << "syscall\n";
    }
}
