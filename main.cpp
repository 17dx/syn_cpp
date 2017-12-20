#include <iostream>
#include <fstream>
#include <string>

using namespace std;
const int TypeUnexpectedChar= -2;
const int TypeError= -1;
const int TypeEmplyLine= 0;
const int TypeAssign= 1;
const int TypeIf= 2;
const int TypeFor= 3;
const int ValidExpresion= 4;
const int ValidOperator= 5;
const int ValidBraket = 6;
const int ValidIf = 7;
const int ValidFor = 8;


struct SResult {
     int code;
     size_t pos;
     string msg;
     SResult (int code):code(code), pos(0){};
     SResult (int code, size_t pos, const char * msg_, char c):
         code(code), pos(pos), msg(msg_){
         msg +='"';
         msg +=  c;
         msg +='"';
     };
     SResult (int code, size_t pos, const char * msg_):
         code(code), pos(pos), msg(msg_){
     };
     void print(int line, int pos_last_break_line){
      cout<< line<< ":" << pos+1-pos_last_break_line
          <<" "<< msg <<endl;
     }
};

SResult checkExpresion(string & s, size_t& i, bool use_logical);
SResult getTypeOperation(string & s, size_t& i );

bool isSpace(char c){
   return (' ' == c || '\t' == c || '\n' == c );
}

bool isAlfa(char c){
   return ('a' <= c && c <='z' )
     || ('A' <= c && c <='Z');
}

bool isNum(char c){
   return ('0' <= c && c <='9' );
}

SResult hasOperator(string & s, size_t& i, bool use_logical){
      while(i< s.size() && isSpace(s[i])) ++i;
      if (i == s.size()) return SResult(TypeError,i, "operator not found");

       char op = s[i];
       if (op == '+' || op =='-') return SResult(ValidOperator);

       while (use_logical){

          if (i+1 == s.size()) break;
          ++i; //заглядываем вперед
          char op_next = s[i];

          if (op == '=' && op_next =='=') return SResult(ValidOperator);
          if (op == '!' && op_next =='=') return SResult(ValidOperator);
          if (op == '>' && op_next =='=') return SResult(ValidOperator);
          if (op == '<' && op_next =='=') return SResult(ValidOperator);
          if (op == '&' && op_next =='&') return SResult(ValidOperator);
          if (op == '|' && op_next =='|') return SResult(ValidOperator);
          --i; //неудачно заглянули

          if (op == '<') return SResult(ValidOperator);
          if (op == '>') return SResult(ValidOperator);
          break;
       }

       return SResult(TypeUnexpectedChar, i, "unexpected char: ", s[i] );
}

bool hasOperand(string & s, size_t& i){

      while(i< s.size() && isSpace(s[i])) ++i;
      if (i == s.size()) return false;
      if (s[i] == ')') return false;  // для случая (i + )
      return true;
}

SResult checkBraket(string & s, size_t& i, bool use_logical){
       ++i;
       SResult res = checkExpresion(s, i, use_logical);

       if ( res.code == TypeError) return res;
       //cout << s[res.pos];
       bool closed_braket = false;
       if ( res.code == TypeUnexpectedChar){
           if(s[res.pos] != ')') return res;

           ++i; //уйти со скобки
           closed_braket = true;

       }

       if(!closed_braket){
           while(i< s.size() && isSpace(s[i])) ++i;
           if (i == s.size()) return SResult(TypeError, i, "miss closed braket");
       }
       else{
           //оператор идет за закрывающейся скобкой
           res =  hasOperator(s, i, use_logical);
           if ( res.code == TypeError) return SResult(ValidBraket);
           if ( res.code != ValidOperator) return res;
           ++i;
           if (!hasOperand(s, i)) return SResult(TypeError,i, "operand not found" );
       }

       return SResult(ValidBraket);
}


SResult checkExpresion(string & s, size_t& i, bool use_logical){
//cout<< "ce: " << s.substr(i)<<  endl;
  while(i< s.size() && isSpace(s[i])) ++i;
  if (i >= s.size()) return SResult(TypeEmplyLine);  // пустое выражение

  if(s[i] == ')') return SResult(TypeError,i, "unexpected closing braket" ); // для пустых скобок

  while(i< s.size() ){

       if(s[i] == '(' ) {
            SResult res = checkBraket(s, i, use_logical);
            if ( res.code != ValidBraket) return res;
       }
       else{
              bool has_name = isAlfa(s[i]);
              bool has_num = isNum(s[i]);
             // cout << s[i];
              ++i;

              if( !has_name && !has_num ) return SResult(TypeUnexpectedChar,i-1, "unexpected char: ", s[i-1] );
              if ( has_name && isAlfa(s[i]) ) return SResult(TypeError,i, "long name" );

              if(has_num){
                //пропускакем числа
                while(i< s.size() && isNum(s[i]) ) ++i;
                if (i == s.size()) return SResult(ValidExpresion);
              }
              SResult res =  hasOperator(s, i, use_logical);
              if ( res.code == TypeError) return SResult(ValidExpresion);
              if ( res.code != ValidOperator) return res;
              ++i;
              if (!hasOperand(s, i)) return SResult(TypeError,i, "operand not found" ); // отсутствует операнд
        }
  }

  return SResult(ValidExpresion);
}


SResult checkIf(string & s, size_t& i){

     while(i< s.size() && isSpace(s[i])) ++i;
     if (i == s.size()) return SResult(TypeError, i, "syntax error in IF");

     if(s[i] != '(' ) return SResult(TypeUnexpectedChar,i, "unexpected char: ", s[i] );
     ++i; //уйти со скобки
     SResult res = checkExpresion(s, i, true);
     if ( res.code == TypeUnexpectedChar){
           // cout << s[i];
           if(s[res.pos] != ')') return res;
           ++i; //уйти со скобки
      }
      else if ( res.code == ValidExpresion
               || res.code == TypeEmplyLine) return SResult(TypeError, i, "miss closed braket");
      else return res ;

      res = getTypeOperation(s, i);

      if ( res.code == TypeAssign) return SResult(ValidIf);
      if ( res.code == TypeEmplyLine) return SResult(TypeError, i, "miss expression after IF ");
      if ( res.code == TypeFor) return SResult(TypeError, i, "forbidden FOR ");
      if ( res.code == TypeIf) return SResult(TypeError, i, "forbidden IF ");

      return res;
}

SResult checkForExpr(string & s, size_t& i){
      SResult res = getTypeOperation(s, i);
      if ( res.code == TypeUnexpectedChar){
           // cout << s[i];
           if(s[res.pos] != ')') return res;
           ++i; //уйти со скобки
      }
      else if ( res.code == TypeError) return res;
      else return SResult(TypeError, i, "miss closed braket");

      res = getTypeOperation(s, i);

      if ( res.code == TypeAssign) return SResult(ValidExpresion);
      if ( res.code == TypeEmplyLine) return SResult(TypeError, i, "miss expression after FOR ");
      if ( res.code == TypeFor) return SResult(TypeError, i, "forbidden FOR ");
      if ( res.code == TypeIf) return SResult(TypeError, i, "forbidden IF ");

     return res;
}

SResult checkForCond(string & s, size_t& i){
     SResult res = checkExpresion(s, i, true);
     if ( res.code == TypeUnexpectedChar){
           if(s[res.pos] == ')') return SResult(TypeError, i, "miss expression-block in FOR");
           return res;
     }
     else if ( res.code == TypeEmplyLine) return SResult(ValidExpresion) ;
     else if ( res.code != ValidExpresion) return res;
     return   SResult(ValidExpresion);
}

SResult checkForInit(string & s, size_t& i){

     SResult res = getTypeOperation(s, i);
     if ( res.code == TypeFor) return SResult(TypeError, i, "forbidden FOR ");
     else if ( res.code == TypeIf) return SResult(TypeError, i, "forbidden IF ");
     else if ( res.code == TypeUnexpectedChar){
           if(s[res.pos] == ')') return SResult(TypeError, i, "miss condition-block in FOR");
           return res;
     }
     else if ( res.code == TypeEmplyLine) return SResult(ValidExpresion) ;
     else if ( res.code != TypeAssign) return res ;

     return SResult(ValidExpresion);
}

SResult checkFor(string & s, size_t& i, ifstream& in){
     while(i< s.size() && isSpace(s[i])) ++i;
     if (i == s.size()) return SResult(TypeError, i, "syntax error in FOR");

     if(s[i] != '(' ) return SResult(TypeUnexpectedChar,i, "unexpected char: ", s[i] );

     ++i; //уйти со скобки

     SResult res =  checkForInit(s,  i);
     if ( res.code != ValidExpresion )  return res;


     if( in.eof() ) return SResult(TypeError, 0, "unexpected end of file " );
     //дочитываем и накапливаем
     string  old = s;
     getline(in, s, ';') ;
     s = old + ";" + s;
     ++i;

     res =  checkForCond(s,  i);
     if ( res.code != ValidExpresion ) return res;

     if( in.eof() ) return SResult(TypeError, 0, "unexpected end of file" );
     //дочитываем и накапливаем
     old = s;
     getline(in, s, ';') ;
     s = old + ";" + s;
     ++i;

     res =  checkForExpr(s,  i);
     if ( res.code != ValidExpresion ) return res;
     return SResult(ValidFor);
}

SResult getTypeOperation(string & s, size_t& i ){

  while(i< s.size() && isSpace(s[i]) ) ++i;
  if (i >= s.size()) return SResult (TypeEmplyLine);

  bool has_name = isAlfa(s[i]) ;

  if(s[i] == '(') return SResult(TypeError,i, "unexpected opening braket" );
  if(s[i] == ')') return SResult(TypeUnexpectedChar,i, "unexpected closing braket" );
  if (! has_name) return SResult (TypeError, i, "bad name" );
  char name = s[i];

  ++i;
  if (i == s.size()) return SResult(TypeError, i, "found only name" );;


  if (name == 'I' || name == 'i'){
    if ( s[i] == 'F' || s[i] == 'f') return SResult(TypeIf);
  }

  if (name == 'F' || name == 'f'){
    if ( s[i] == 'O' || s[i] == 'o'){
        ++i;
        if (i == s.size()) return SResult(TypeError, i, "not valid name" );
        if ( s[i] == 'R' || s[i] == 'r')  return SResult(TypeFor);
        else return SResult(TypeError, i, "not valid name" );
    }
  }
  if ( isAlfa(s[i]) ) return SResult(TypeError, i, "long name" );

  while(i< s.size() && isSpace(s[i])) ++i;
  if (i == s.size()) return SResult(TypeError, i, "found only name" );  // отсутствует символ =
  if (s[i] != '=') return SResult(TypeUnexpectedChar, i, "unexpected char: ", s[i] );

  ++i;
  SResult res = checkExpresion(s, i, false);
  if ( res.code == TypeEmplyLine) return SResult(TypeError, i, "not valid expression");
  if ( res.code == TypeError || res.code == TypeUnexpectedChar) return res;

  return SResult(TypeAssign);
}

void treatmentError(SResult& res, int nline, string& s){
    size_t pos_last_break_line =-1; // для первой строки
    for(size_t i =0; i< res.pos; ++i)
        if (s[i]=='\n'){
           ++nline;
           pos_last_break_line = i;
        }

    res.print(nline,pos_last_break_line );
    cout<<'"'<< s.substr(pos_last_break_line+1)<< '"'<< endl;
    cout<<  string( res.pos-pos_last_break_line , ' ') << "^" <<endl; // указатель на ошибку
}

int main()
{
  ifstream in("1.txt");
  if (!in.is_open() ) {
    cout<< "file not open!";
    return 0;
  };
  int nline=1;
  while(! in.eof()){
    string s;
    getline(in, s, ';') ;
    //cout<< s<< endl;
    size_t i=0;
    SResult res = getTypeOperation(s, i);

    if (( res.code == TypeError || res.code == TypeUnexpectedChar)){
       treatmentError(res, nline, s );
    }

    else if ( res.code == TypeIf){
        ++i;
        SResult res =  checkIf(s,  i);
        if (( res.code == TypeError || res.code == TypeUnexpectedChar)){
           treatmentError(res, nline, s );
        }
    }
    else if ( res.code == TypeFor){
        ++i;
        SResult res =  checkFor(s,  i, in);
        if (( res.code == TypeError || res.code == TypeUnexpectedChar)){
           treatmentError(res, nline, s );
        }
    }

    for(size_t i =0; i< s.size(); ++i)
       if (s[i]=='\n') ++nline;

  }
  in.close();
  cin.get();

  return 0;
}

