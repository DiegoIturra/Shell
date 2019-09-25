#include <iostream>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <vector>
#include <signal.h>
#include <ctime>
#include <sstream>
#include <cstdlib>
#include <map>
#include <fstream>
#include <sstream>
#include <iterator>

using namespace std;

string getDate();
void color_(int);
void escribirComandos(const string&, ofstream&, string, string);
void crearArchivo(string, ofstream&);
void leerComandos(const string&);
char** get_input(char*);
vector<char**> pipe_comm(char*);
void doPipe(vector<char**>);
int check_input(char*);
void sleep_comm(int);
void signal_comm(char*);
void info(string);
void impEjecComando(map<int,string>);
void entradas(const string&, char**, vector<char**>, int, char*, map<string,string>, map<int,string>);
void desplegarMenu(const string&, map<string,string>, map<int,string>);


struct timespec t1, t2, t3;
bool flag;


int main(){
	int status;
	vector<char**> vec;
	char** command;
	pid_t pid;
	/*realcionado con archivo*/
	ofstream file; //archivo de tipo de escritura
	string nombreArchivo = "mishell.log";
    	crearArchivo(nombreArchivo, file);
    	escribirComandos(nombreArchivo, file, getDate(), "Inicio sesion Shell");

	map<string,string> infoComando;
	map<int, string> ejecComando;
	int cont = 1;


	//bucle de la shell
	while(1){
		cout << "\033[1;91mMyShell~$ \033[0m";
		color_(0);
		char input[100];
		cin.getline(input, 100);
		//para pasarlos al archivo
		string fecha = getDate();
		string comando = string(input);
		escribirComandos(nombreArchivo,file,fecha,comando);
		infoComando[comando] = fecha;
		ejecComando[cont] = comando;
		cont++;
		//mapear
		entradas(nombreArchivo, command, vec, status, input, infoComando, ejecComando);
	}
	free(command);
	return 0;
}



/*----------------- NUEVAS FUNCIONES PARA FECHA Y ARCHIVOS --------------------------------*/
string getDate(){
    time_t t = time(NULL);
    //puntero a esteructura de tipo tm (propio de la libreria ctime)
    tm* timePtr = localtime(&t);

    /*convertir a string todos los datos anio/mes/dia hora/minuto/segundo*/
    stringstream year; //se usa stringstream para dirigir flujos en memoria
    year << timePtr->tm_year+1900; // luego se redirige la info sobre "year"
    string Year = year.str(); //se castea el stringStream a string

    stringstream month;
    month << timePtr->tm_mon+1;
    string Month = month.str();
    if(atoi(Month.c_str()) < 10)
        Month = "0"+Month;

    stringstream day;
    day << timePtr->tm_mday;
    string Day = day.str();
    if(atoi(Day.c_str()) < 10)
        Day = "0"+Day;

    stringstream hour;
    hour << timePtr->tm_hour;
    string Hour = hour.str();
    if(atoi(Hour.c_str()) < 10)
        Hour = "0"+Hour;

    stringstream minute;
    minute << timePtr->tm_min;
    string Min = minute.str();
    if(atoi(Min.c_str()) < 10)
        Min = "0"+Min;

    stringstream second;
    second << timePtr->tm_sec;
    string Sec = second.str();
    if(atoi(Sec.c_str()) < 10)
        Sec = "0"+Sec;

    string Fecha = Year + "/"+ Month + "/" + Day + " " + Hour + ":" + Min + ":" + Sec;

    return Fecha;
}

void color_(int n){
	srand(time(NULL));
	string colors[] = {"1;35m", "1;36m", "1;37m", "1;38m","1;96m"};
	int random = rand()%5;
	cout << "\e[" + colors[random];
}

void escribirComandos(const string& nombreFichero , ofstream& fichero , string fecha, string comando){
    fichero.open(nombreFichero.c_str(),ios::app); //setea archivo desde la ultima linea en la que quedo
    fichero << fecha << " " << comando << endl;
    fichero.close();
}


void crearArchivo(string nombreFichero , ofstream& fichero){
    string name = nombreFichero;
    ofstream dummy(name.c_str()); //crea fichero con nombre dado
}

void leerComandos(const string& nombreFichero){
    ifstream archivoLectura;
    archivoLectura.open(nombreFichero.c_str() , ios::in); //abre archivo modo entrada
    string texto;
    while(!archivoLectura.eof()){
        getline(archivoLectura,texto);
        cout << texto << endl;
    }
}

char** get_input(char* input){
	char** command = (char**)malloc(5*sizeof(char*));
	char separator[] = " ";
	char* parsed = strtok(input, separator);
	int index = 0;

	while(parsed != NULL){
		command[index] = parsed;
		index++;
		parsed = strtok(NULL, separator);
	}
	command[index] = NULL;
	return command;
}

//ve si el comando queentra posee un pipe
vector<char**> pipe_comm(char* input){
	char** command = (char**)malloc(5*sizeof(char*));
	char separator[] = "|";
	char* parsed = strtok(input, separator);
	int index = 0;

	while(parsed != NULL){
		command[index] = parsed;
		index++;
		parsed = strtok(NULL, separator);
	}
	command[index] = NULL;
	vector<char**> vec;
	for(int i=0 ; i<index ; i++) vec.push_back(get_input(command[i]));

	return vec;
}

void doPipe(vector<char**> cmd){
	pid_t pid;
	int status1;
	int fd[2];
	int size = cmd.size();
	int i = 0;

	int fds = 0;
	while(size--){

		char** inst = cmd[i]; //recibir el comando
		pipe(fd); //crear un pipe
	
		pid = fork(); //hacer un proceso hijo
		if(pid < 0) exit(-1);
		if(pid == 0){ //se crea el hijo
			dup2(fds, STDIN_FILENO); //duplicar el descriptor de lectura
			if(size > 0){
				dup2(fd[1], STDOUT_FILENO); //si quedan mas comandos por ejecutar
			}
			close(fd[0]);
			execvp(inst[0], inst);
			exit(1);
		}else{
			/*status guarda el estado del hijo , si termino , si no , etc*/
			waitpid(pid, &status1, WUNTRACED);	//significa que también vuelve si hay hijos parados (pero no  rastreados)
			close(fd[1]);
			fds = fd[0];
		}
		
		i++;
	}
}

int check_input(char* input){
	for(int i=0 ; i<strlen(input) ; i++){
		if(input[i] == '|') return 1;
	}
	if(strncmp(input, "miocio", 6) == 0) return 2;
	if(strncmp(input, "info", 4) == 0) return 3;
	else return 0;
}

void sleep_comm(int sig){
	if(sig == SIGUSR1){
		cout << "TIEMPO DURMIENDO: " << endl;
		cout << t1.tv_sec - t2.tv_sec << " segundos." <<endl;
		if(flag){ //si, continua esperando por comandos
			return;
		}else{ //no, continua durmiendo
			nanosleep(&t2, &t3);
			exit(1);
		}
	}
}

//funcion solo para miocio
void signal_comm(char* input){
	char** command = get_input(input);
	char* arg1_c = command[1];
	int n = strlen(arg1_c)-2;
	arg1_c[n] = '\0';
	char* arg2 = command[2];
	int arg1 = atoi(arg1_c);

	if(strcmp(arg2, "si") == 0) flag = true;
	else flag = false;

	struct sigaction f;
	f.sa_handler = sleep_comm;
	sigaction(SIGUSR1, &f, NULL); /*NULL para no guardar la accion anterior al ejecutar la senal*/

	long nanosec = 0;
	int sec = arg1 / 1000;

	t1.tv_sec =  sec;
	t1.tv_nsec = nanosec;
	//si no ha sido interrumpido , debe terminar
	if(nanosleep(&t1, &t2) != -1) exit(1);
	else return;
}

void info(string nombreArchivo){
	leerComandos(nombreArchivo);
}

void impEjecComando(map<int,string> ejecComando){
	map<int,string>::iterator it;
	for(it = ejecComando.begin() ; it != ejecComando.end() ; it++){
		cout << it->first << " " << it->second << endl;
	}
}

void entradas(const string& nombreArchivo, char** command, vector<char**> vec, int status, char* input, map<string, string> infoComando, map<int,string> ejecComando){
	int check = check_input(input);

	if(check == 1){ //lee un pipe
		vec = pipe_comm(input);
		doPipe(vec);
	}else if (check == 2){ //es con miocio
		signal_comm(input);
	}else if(check == 3){ //es con con info
		desplegarMenu(nombreArchivo, infoComando, ejecComando);
	}else{ //comandos normales
		command = get_input(input);
		int pid = fork();
		if(pid == 0){
			execvp(command[0], command);
			perror(input);
			exit(-1);
		}else{
			waitpid(pid, &status, WUNTRACED);
		}
		free(command);
	}
}


void desplegarMenu(const string& nombreFichero, map<string, string> infoComando, map<int,string> ejecComando){
	int opc;
	color_(1);
	cout << ".: Menú :. " << endl << endl;
	cout << "1. Buscar comando" << endl;
	cout << "2. Desplegar comandos utilizados" << endl;
	cout << "3. Elegir comando" << endl << endl;
	cin >> opc;
	cout << endl;
	cin.ignore();
	switch(opc){
		case 1: 
		{
			cout << "Escriba comando: ";
			char command[30];
			cin.getline(command,30);
			map<string,string>::iterator it;
			it = infoComando.find((string)command);
			color_(2);
			if(it != infoComando.end()) cout << it->first << " " << it->second << endl;
			else cout << "Comando no encontrado" << endl;

		}
		break;
		case 2:
		{
			color_(2);
			leerComandos(nombreFichero);
		}
		break;
		case 3:
		{
			impEjecComando(ejecComando);
			color_(3);
			cout << endl;
			cout << "Escoja numero de comando a ejecutar: ";
			int command;
			cin >> command;
			map<int,string>::iterator it;
			it = ejecComando.find(command);
			cout << endl;
			char* instr;
			if(it != ejecComando.end()){
				instr = strcpy(instr, (it->second).c_str());
			}
			else cout << "Comando no encontrado" << endl;
			cin.ignore();
			char** cmd;
			vector<char**> vec;
			int status;
			entradas(nombreFichero, cmd, vec, status, instr, infoComando, ejecComando);
		}
		break;
		default: cout << "Opcion no valida" << endl;
		break;
	}
}
