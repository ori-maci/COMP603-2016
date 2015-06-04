/*
= Brainfuck

If you have gcc:

----
g++ -o brainfuck.exe brainfuck.cpp
brainfuck.exe helloworld.bf
----
*/

#include <vector>
#include <iostream>
#include <fstream>

using namespace std;

/**
 * Primitive Brainfuck commands
 */
typedef enum { 
    INCREMENT, // +
    DECREMENT, // -
    SHIFT_LEFT, // <
    SHIFT_RIGHT, // >
    INPUT, // ,
    OUTPUT, // .
	ZERO // [-] or [+]
} Command;

// Forward references. Silly C++!
class CommandNode;
class Loop;
class Program;

/**
 * Visits?!? Well, that'd indicate visitors!
 * A visitor is an interface that allows you to walk through a tree and do stuff.
 */
class Visitor {
    public:
        virtual void visit(const CommandNode * leaf) = 0;
        virtual void visit(const Loop * loop) = 0;
        virtual void visit(const Program * program) = 0;
};

/**
 * The Node class (like a Java abstract class) accepts visitors, but since it's pure virtual, we can't use it directly.
 */
class Node {
    public:
        virtual void accept (Visitor *v) = 0;
};

/**
 * CommandNode publicly extends Node to accept visitors.
 * CommandNode represents a leaf node with a primitive Brainfuck command in it.
 */
class CommandNode : public Node {
    public:
        Command command;
		int count;
        CommandNode(char c, int n) {
            switch(c) {
                case '+': command = INCREMENT; break;
                case '-': command = DECREMENT; break;
                case '<': command = SHIFT_LEFT; break;
                case '>': command = SHIFT_RIGHT; break;
                case ',': command = INPUT; break;
                case '.': command = OUTPUT; break;
				case 'z': command = ZERO; break;
            }
			count = n;
        }
        void accept (Visitor * v) {
            v->visit(this);
        }
};

class Container: public Node {
    public:
        vector<Node*> children;
        virtual void accept (Visitor * v) = 0;
};

/**
 * Loop publicly extends Node to accept visitors.
 * Loop represents a loop in Brainfuck.
 */
class Loop : public Container {
    public:
        void accept (Visitor * v) {
            v->visit(this);
        }
};




/**
 * Program is the root of a Brainfuck program abstract syntax tree.
 * Because Brainfuck is so primitive, the parse tree is the abstract syntax tree.
 */
class Program : public Container {
    public:
        void accept (Visitor * v) {
            v->visit(this);
        }
};

/**
 * Read in the file by recursive descent.
 * Modify as necessary and add whatever functions you need to get things done.
 */
void parse(fstream & file, Container * container) {
	Loop * program; // Our loop object
	char c;
	int count;
    // How to insert a node into the container

	while (file >> c) {
		count = 1; // reset count
		//command case
		if(c == '+' || c == '-' || c == '<' || c == '>' || c == ',' || c == '.'){
			while(((char)file.peek()) == c){ // Squash down repeats, aka +++ -> Node with + and count = 3
				file >> c; // move file pointer
				count++; // increase
			}
			container->children.push_back(new CommandNode(c,count)); //add our node to the tree
		}
		else if(c == '['){  // Loop case
			program = new Loop(); // Create new loop object
			parse(file, program); // Parse the inside of the loop
			if (program->children.size() == 1) { // If we have only one object inside the loop, check for special cases.
				CommandNode* child = dynamic_cast<CommandNode*>(program->children.front());  // 
				if (child->command == INCREMENT || child->command == DECREMENT) { // If loop is either [+] or [-]
					container->children.push_back(new CommandNode('z',1)); // Add special ZERO node.
					delete program; // Avoid lingering loop objects
				} else {
					container->children.push_back(program);	// Normal non-special loop with size one.
				}
			}else{
				container->children.push_back(program);	 //Add a normal non-special loop to the tree.
			}
		}
		else if (c == ']') { //end of loop object
			return;
		}
		
	}
}


/*
Program -> Sequence

Sequence -> Command Sequence
Sequence -> Loop Sequence
Sequence -> any other character, ignore (treat as a comment)
Sequence -> "" (empty string)

Command -> '+' | '-' | '<' | '>' | ',' | '.'

Loop -> '[' Sequence ']'
*/

/**
 * A printer for Brainfuck abstract syntax trees.
 * As a visitor, it will just print out the commands as is.
 * For Loops and the root Program node, it walks trough all the children.
 */
class Printer : public Visitor {
    public:
        void visit(const CommandNode * leaf) {
		for (int i = 0; i < leaf->count; i++){
				switch (leaf->command) {
					case INCREMENT:   cout << '+'; break;
					case DECREMENT:   cout << '-'; break;
					case SHIFT_LEFT:  cout << '<'; break;
					case SHIFT_RIGHT: cout << '>'; break;
					case INPUT:       cout << ','; break;
					case OUTPUT:      cout << '.'; break;
					case ZERO:		  cout << 'z'; break;
				}
			}
        }
        void visit(const Loop * loop) {
            cout << '[';
            for (vector<Node*>::const_iterator it = loop->children.begin(); it != loop->children.end(); ++it) {
                (*it)->accept(this);
            }
            cout << ']';
        }
        void visit(const Program * program) {
            for (vector<Node*>::const_iterator it = program->children.begin(); it != program->children.end(); ++it) {
                (*it)->accept(this);
            }
            cout << '\n';
        }
};

class JavaCompiler : public Visitor {
    public:
        void visit(const CommandNode * leaf) {
			for (int i = 0; i < leaf->count; i++){
					switch (leaf->command) {
						case INCREMENT:   cout << "array[pointer]++;\n"; break;
						case DECREMENT:   cout << "array[pointer]--;\n"; break;
						case SHIFT_LEFT:  cout << "pointer--;\n"; break;
						case SHIFT_RIGHT: cout << "pointer++;\n"; break;
						case INPUT:       cout << "array[pointer] = (byte)System.in.read();\n"; break;
						case OUTPUT:      cout << "System.out.print((char)array[pointer]);\n"; break;
						case ZERO:		  cout << "array[pointer]=0;\n"; break;
					}
			}
        }
        void visit(const Loop * loop) {
            cout << "while (array[pointer] == 1){ \n";
            for (vector<Node*>::const_iterator it = loop->children.begin(); it != loop->children.end(); ++it) {
                (*it)->accept(this);
            }
            cout << "}\n";
        }
        void visit(const Program * program) {
			cout << "import java.util.Scanner;\n";
			cout << "import java.io.IOException;\n\n";
			cout << "public class Default {\n";
			cout << "public static void main(String[] args) throws IOException {\n";
			cout << "Scanner input = new Scanner(System.in);\n";
			cout << "byte[] array = new byte[30000];\n";
			cout << "int pointer = 0;\n";
            for (vector<Node*>::const_iterator it = program->children.begin(); it != program->children.end(); ++it) {
                (*it)->accept(this);
            }
			cout << "}\n";
            cout << "}\n";
        }
};

class Interpreter : public Visitor {
    char memory[30000];
    int pointer;
    public:
        void visit(const CommandNode * leaf) {
			for (int i = 0; i < leaf->count; i++){
				switch (leaf->command) {
					case INCREMENT:
						memory[pointer]++;
						break;
					case DECREMENT:
						memory[pointer]--;
						break;
					case SHIFT_LEFT:
						pointer--;
						break;
					case SHIFT_RIGHT:
						pointer++;
						break;
					case INPUT:
						cin.get(memory[pointer]);
						break;
					case OUTPUT:
						cout << memory[pointer];
						break;
					case ZERO:
						memory[pointer]=0;
						break;
				}
			}
        }
        void visit(const Loop * loop) {
			while(memory[pointer]){
				for (vector<Node*>::const_iterator it = loop->children.begin(); it != loop->children.end(); ++it) {
					(*it)->accept(this);
				}
			}
        }
        void visit(const Program * program) {
            // zero init the memory array
            // set pointer to zero
			for(int i =0; i <= 30000; i++){
				memory[i] = 0;
			}
			pointer = 0;

            for (vector<Node*>::const_iterator it = program->children.begin(); it != program->children.end(); ++it) {
                (*it)->accept(this);
            }
        }
};

int main(int argc, char *argv[]) {
    fstream file;
    Program program;
    Printer printer;
	JavaCompiler compiler;
    Interpreter interpreter;
    if (argc == 1) {
        cout << argv[0] << ": No input files." << endl;
    } else if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            file.open(argv[i], fstream::in);
            parse(file, & program);
         //  program.accept(&printer);
           program.accept(&interpreter);
		 //	program.accept(&compiler);
            file.close();
        }
    }
}