/*******************************************************************************/
/*									       */
/*				     tron5.c				       */
/*									       */
/*  Programa corresponent a la pràctica 2.2 de FSO.	                       */
/*     Es tracta del joc del tron: sobre un camp de joc rectangular, es        */
/*     mouen uns objectes que anomenarem 'trons' (amb o tancada). En aquesta   */
/*     quarta versio del joc, nomes hi ha un tron que controla l'usuari, i     */
/*     que representarem amb un '0', i diversos trons que controla l'ordinador,*/ 
/*     el  qual es representara amb un número del '1' al '9'. Els trons son    */
/*     una especie de 'motos' que quan es mouen deixen rastre (el caracter     */
/*     corresponent). L'usuari  pot canviar la direccio de moviment del seu    */ 
/*     tron amb les tecles: 'w' (adalt), 's' (abaix), 'd' (dreta) i 'a'        */
/*     (esquerra). El trons que controla l'ordinador es mouran aleatoriament,  */
/*     canviant de direcció aleatoriament segons un parametre del programa     */
/*     (veure Arguments). El joc consisteix en que un tron intentara 'tancar'  */
/*     a l'altre tron. El primer tron que xoca contra un obstacle (sigui       */
/*     rastre seu o de l'altre tron), esborrarà tot el seu rastre i perdrà la  */ 
/*     partida.       							       */
/*									       */
/*  Arguments del programa:						       */
/*     El primer argument serà el número de oponents que es volen crear.       */
/*     Per controlar la variabilitat del canvi de direccio, s'ha de propor-    */
/*     cionar com a segon argument un número del 0 al 3, el qual indicara      */
/*     si els canvis s'han de produir molt frequentment (3 es el maxim) o      */
/*     poc frequentment, on 0 indica que nomes canviarà per esquivar les       */
/*     parets.								       */
/*     Hi haura un tercer parametre que sera el nom d'un fitxer on guardarem   */
/*     les llargaries dels diferents trons.				       */
/*     A mes, es podra afegir un quart argument opcional per indicar el        */
/*     retard de moviment dels diferents trons (en ms);                        */
/*     el valor per defecte d'aquest parametre es 100 (1 decima de segon).     */
/*									       */
/*  Compilar i executar:					  	       */
/*     El programa invoca les funcions definides a "winsuport2.c", les         */
/*     quals proporcionen una interficie senzilla per crear una finestra       */
/*     de text on es poden escriure caracters en posicions especifiques de     */
/*     la pantalla (basada en CURSES); per tant, el programa necessita ser     */
/*     compilat amb la llibreria 'curses':				       */
/*									       */
/*	   $ gcc -c winsuport2.c -o winsuport2.o			       */
/*	   $ gcc tron5.c winsuport2.o -o tron5 -lcurses -lpthread	       */
/*	   $ ./tron5 num_oponents variabilitat fitxer [retard] 		       */
/*									       */
/*  Codis de retorn:						  	       */
/*     El programa retorna algun dels seguents codis al SO:		       */
/*	0  ==>  funcionament normal					       */
/*	1  ==>  numero d'arguments incorrecte 				       */
/*	2  ==>  no s'ha pogut crear el camp de joc (no pot iniciar CURSES)     */
/*	3  ==>  no hi ha prou memoria per crear les estructures dinamiques     */
/*									       */
/*******************************************************************************/

		
#include <stdio.h>		/* incloure definicions de funcions estandard */		
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "winsuport2.h"		/* incloure definicions de funcions propies */
#include "memoria.h"
#include "semafor.h"
#include "missatge.h"

#define MAX_OPONENTS 9

				/* definir estructures d'informacio */
typedef struct {		/* per un tron (usuari o oponent) */
	int f;				/* posicio actual: fila */
	int c;				/* posicio actual: columna */
	int d;				/* direccio actual: [0..3] */
} tron;

typedef struct {		/* per una entrada de la taula de posicio */
	int f;
	int c;
} pos;


	/* variables globals */
pid_t tpid_opo[MAX_OPONENTS];	/* Taula d'indentificadors dels threads dels oponents */
pthread_t id_usu;		/* Identificador del thread de l'ususari */
pthread_t id_temps;		/* Identificador del thread encarregat de l'actualització del temps */

int n_fil, n_col;		/* dimensions del camp de joc */

tron usu;   	   		/* informacio de l'usuari */
tron opo[MAX_OPONENTS];		/* informacio deLS oponents */

int df[] = {-1, 0, 1, 0};	/* moviments de les 4 direccions possibles */
int dc[] = {0, -1, 0, 1};	/* dalt, esquerra, baix, dreta */

int varia;		/* valor de variabilitat dels oponents [0..9] */
FILE *fitxer;           /* descriptor de fitxer on escriurem les longituts */
int retard;		/* valor del retard de moviment, en mil.lisegons */

pos *p_usu;			/* taula de posicions que van recorrent els jugadors */
int n_usu = 0;		/* numero d'entrades en les taules de pos. */
char dia[9];
char hora[9];

int *fi1;
int n_oponents, *oponents_morts;
int id_bustia_xocs, id_bustia_morts;
int invers, mort;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;



/* funcio per esborrar totes les posicions anteriors, sigui de l'usuari o */
/* de l'oponent */
void esborrar_posicions(char car_tron, pos p_pos[], int n_pos)
{
	int i;
	char cars;

	fprintf(fitxer, "%s %s tron acabat %c: %d\n",dia,hora,car_tron,n_pos);
	//fprintf(stderr, "esborrant %d posicions!\n",n_pos);
	for (i=n_pos-1; i>=0; i--)		/* de l'ultima cap a la primera */
	{
		//fprintf(stderr, "esborrant posicio %d: (%d,%d)\n",i,p_pos[i].f,p_pos[i].c);
		pthread_mutex_lock(&mutex);
		cars = win_quincar(p_pos[i].f,p_pos[i].c);
		if (cars == '0')
			win_escricar(p_pos[i].f,p_pos[i].c,' ',NO_INV);	/* esborra una pos. */
		if (*oponents_morts == n_oponents)
			win_update();
		pthread_mutex_unlock(&mutex);
		win_retard(10);		/* un petit retard per simular el joc real */
	}
}



/* funcio per inicialitar les variables i visualitzar l'estat inicial del joc */
void inicialitza_joc()
{
  char strin[45];

  usu.f = (n_fil-1)/2;
  usu.c = (n_col)/4;		/* fixa posicio i direccio inicial usuari */
  usu.d = 3;
  pthread_mutex_lock(&mutex);
  win_escricar(usu.f,usu.c,'0',INVERS);	/* escriu la primer posicio usuari */
  pthread_mutex_unlock(&mutex);
  p_usu[n_usu].f = usu.f;		/* memoritza posicio inicial */
  p_usu[n_usu].c = usu.c;
  n_usu++;

  sprintf(strin,"Tecles: \'%c\', \'%c\', \'%c\', \'%c\', RETURN-> sortir\n",
		TEC_AMUNT, TEC_AVALL, TEC_DRETA, TEC_ESQUER);
  win_escristr(strin);
}

/* funció per obtenir els missatges de la bustia */
void * comprovar_bustia_xocs(void * nulo)
{
  char missatge[2];

  do 
  {
	receiveM(id_bustia_xocs, missatge);
	if (*missatge == '0') {
		invers = 0;
		win_retard(10000);
		invers = 1;
	}
	else
		sendM(id_bustia_xocs, missatge, 2);	
  } while (!(*fi1) && (*oponents_morts < n_oponents));
  return ((void *)(intptr_t) 0);
}

/* funció per obtenir el missatge de la bustia */
void * comprovar_bustia_morts(void * nulo)
{
  char missatge[2];

  do
  {
	receiveM(id_bustia_morts, missatge);
	if (*missatge == '0')
		mort = 1;
	else
		sendM(id_bustia_morts, missatge, 2);	
  } while (!(*fi1) && (*oponents_morts < n_oponents));
  return ((void *)(intptr_t) 0);
}

/* funcio per moure l'usuari una posicio, en funcio de la direccio de   */
/* moviment actual; retorna -1 si s'ha premut RETURN, 1 si ha xocat     */
/* contra alguna cosa, i 0 altrament */
void * mou_usuari(void * nulo)
{
  char cars, missatge[2];
  int atrib;
  tron seg;
  int tecla;
  pthread_t  id_comprovar_bustia_xocs, id_comprovar_bustia_morts;

  pthread_create(&id_comprovar_bustia_xocs, NULL, comprovar_bustia_xocs, (void *)(intptr_t) 0);
  pthread_create(&id_comprovar_bustia_morts, NULL, comprovar_bustia_morts, (void *)(intptr_t) 0);
  
  do 
  {
	pthread_mutex_lock(&mutex);
	tecla = win_gettec();
	pthread_mutex_unlock(&mutex);
  	if (tecla != 0)
   		switch (tecla)	/* modificar direccio usuari segons tecla */
   		{
    			case TEC_AMUNT:		usu.d = 0; break;
    			case TEC_ESQUER:	usu.d = 1; break;
   			case TEC_AVALL:		usu.d = 2; break;
    			case TEC_DRETA:		usu.d = 3; break;
    			case TEC_RETURN:	*fi1 = -1; break;
  		}
 	seg.f = usu.f + df[usu.d];	/* calcular seguent posicio */
  	seg.c = usu.c + dc[usu.d];
	pthread_mutex_lock(&mutex);
  	cars = win_quincar(seg.f,seg.c);	/* calcular caracter seguent posicio */
	atrib = win_quinatri(seg.f, seg.c);
 	if (((cars == ' ') || (atrib == 0)) && (mort == 0))			/* si seguent posicio lliure */
  	{
		if (atrib == 0)
		{
			sprintf(missatge,"%c",cars);
			sendM(id_bustia_morts,missatge,2);
		}
    		usu.f = seg.f; 		/* actualitza posicio */
		usu.c = seg.c;
		if (invers)		
    			win_escricar(usu.f,usu.c,'0',INVERS);	/* dibuixa bloc usuari */
		else
			win_escricar(usu.f,usu.c,'0',NO_INV);
		pthread_mutex_unlock(&mutex);
    		p_usu[n_usu].f = usu.f;		/* memoritza posicio actual */
    		p_usu[n_usu].c = usu.c;
    		n_usu++;
  	}
  	else
	{
		sprintf(missatge,"%c",cars);
		sendM(id_bustia_xocs,missatge,2);
		pthread_mutex_unlock(&mutex); 
		esborrar_posicions('0', p_usu, n_usu);
		*fi1 = 1;
	}
	win_retard(retard);
  } while (!(*fi1) && (*oponents_morts < n_oponents));
  return ((void *)(intptr_t) 0);
}

/* funció per controlar el temps de joc */
void * actualitza_temps(void * nulo)
{
  int minuts, segons;
  char strin[45];

  minuts = 0;
  segons = 0;
  do {
  	sprintf(strin,"Temps de joc: %i:%i", minuts, segons);
	pthread_mutex_lock(&mutex);
  	win_escristr(strin);
	pthread_mutex_unlock(&mutex);
	win_retard(1000);		
	segons++;
	if (segons == 60) 
	{
		segons=0;
		minuts++;
	}
  } while (!(*fi1) && (*oponents_morts < n_oponents));
  return ((void *)(intptr_t) 0);
}


/* programa principal				    */
int main(int n_args, const char *ll_args[])
{
  int n, i, mida, id_camp, id_fi1, id_oponents_morts, *p_camp, index, id_sem;		/* variables locals */
  char a1[20], a2[20], a3[20], a4[20], a5[20], a6[20], a7[20], a8[20], a9[20], a10[20], a11[20], a12[20];

  time_t tiempo = time(0);
  struct tm *tlocal = localtime(&tiempo);
  strftime(dia,9,"%d/%m/%y",tlocal);
  strftime(hora,9,"%H:%M:%S",tlocal);

  srand(getpid());		/* inicialitza numeros aleatoris */

  if (n_args <= 4)
  {	fprintf(stderr,"Comanda: ./tron5 num_oponents variabilitat fitxer [retard]\n");
	fprintf(stderr,"	 on \'num_oponents\' indica el nombre d'oponents del joc\n");
  	fprintf(stderr,"         on \'variabilitat\' indica la frequencia de canvi de direccio\n");
  	fprintf(stderr,"         de l'oponent: de 0 a 3 (0- gens variable, 3- molt variable),\n");
  	fprintf(stderr,"         fitxer es el fitxer on acumularem la llargaria dels drons ),\n");
  	fprintf(stderr,"         i \'retard\' es el numero de mil.lisegons que s'espera entre dos\n");
  	exit(1);
  }

  n_oponents=atoi(ll_args[1]);
  if (n_oponents < 1) n_oponents = 1;
  if (n_oponents > 9) n_oponents = 9;

  varia = atoi(ll_args[2]);	/* obtenir parametre de variabilitat */
  if (varia < 0) varia = 0;	/* verificar limits */
  if (varia > 3) varia = 3;

  fitxer = fopen(ll_args[3],"a");
  if (n_args == 5)		/* si s'ha especificat parametre de retard */
  {	
	retard = atoi(ll_args[4]);	/* convertir-lo a enter */
  	if (retard < 10) retard = 10;	/* verificar limits */
  	if (retard > 1000) retard = 1000;
  }
  else retard = 100;		/* altrament, fixar retard per defecte */

  printf("Joc del Tron\n\tTecles: \'%c\', \'%c\', \'%c\', \'%c\', RETURN-> sortir\n",
		TEC_AMUNT, TEC_AVALL, TEC_DRETA, TEC_ESQUER);
  printf("prem una tecla per continuar:\n");
  getchar();

  n_fil = 0;			/* demanarem dimensions de taulell maximes */
  n_col = 0;		
  mida = win_ini(&n_fil,&n_col,'+',INVERS);	/* intenta crear taulell */

  if (mida < 0)	/* si no pot crear l'entorn de joc amb les curses */
  { 
	fprintf(stderr,"Error en la creacio del taulell de joc:\t");
	switch (mida)
	{	
		case -1: fprintf(stderr,"camp de joc ja creat!\n"); break;
		case -2: fprintf(stderr,"no s'ha pogut inicialitzar l'entorn de curses!\n"); break;
		case -3: fprintf(stderr,"les mides del camp demanades son massa grans!\n"); break;
		case -4: fprintf(stderr,"no s'ha pogut crear la finestra!\n"); break;
	}
	exit(2);
  }
  
  p_usu = calloc(n_fil*n_col/2, sizeof(pos));	/* demana memoria dinamica */
  if (!p_usu)	/* si no hi ha prou memoria per als vectors de pos. */
  { 
	win_fi();				/* tanca les curses */
	fprintf(stderr,"Error en alocatacion de memoria dinamica.\n");
	exit(3);
  }
			/* Fins aqui tot ha anat be! */
  id_camp = ini_mem(mida);
  p_camp = map_mem(id_camp);
  win_set(p_camp, n_fil, n_col);

  id_fi1 = ini_mem(sizeof(int));
  fi1 = map_mem(id_fi1);
  *fi1 = 0;

  id_oponents_morts = ini_mem(sizeof(int));
  oponents_morts = map_mem(id_oponents_morts);
  *oponents_morts = 0; 

  inicialitza_joc();
  pthread_mutex_init(&mutex, NULL);
  id_sem=ini_sem(1);
  id_bustia_xocs=ini_mis();
  id_bustia_morts=ini_mis();

  invers = 1;
  mort = 0;
  n = 0;
  for (i = 0; i < n_oponents; i++) 
  {
	tpid_opo[n] = fork();
	if (tpid_opo[n] == (pid_t) 0) 
	{
		sprintf(a1,"%i",id_camp);
		sprintf(a2,"%i",n_fil);
		sprintf(a3,"%i",n_col);
		sprintf(a4,"%i",varia);
		sprintf(a5,"%i",retard);
		sprintf(a6,"%i",id_fi1);
		sprintf(a7,"%i",id_oponents_morts);
		sprintf(a8,"%i",i);
		sprintf(a9,"%i",n_oponents);
		sprintf(a10,"%i",id_sem);
		sprintf(a11,"%i",id_bustia_xocs);
		sprintf(a12,"%i",id_bustia_morts);
		execlp("./oponent5", "oponent5", a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, ll_args[3], (char *) 0);
		fprintf(stderr,"Error: No es pot executar el procés fill \'oponent6\'\n");
		fclose(fitxer);
  		elim_mem(id_camp);
  		elim_mem(id_fi1);
  		elim_mem(id_oponents_morts);
		pthread_mutex_destroy(&mutex);
		elim_sem(id_sem);
		elim_mis(id_bustia_xocs);
		elim_mis(id_bustia_morts);
  		free(p_usu);
		exit(0);	
	}
	else if (tpid_opo[n] > 0) n++;
  }
  pthread_create(&id_usu, NULL, mou_usuari, (void *)(intptr_t) 0);
  pthread_create(&id_temps, NULL, actualitza_temps, (void *)(intptr_t) 0);

  do 
  {		// bucle principal
	pthread_mutex_lock(&mutex);
	win_update();
	pthread_mutex_unlock(&mutex);
	win_retard(100);	
  } while (!(*fi1) && (*oponents_morts < n_oponents));

  for (i = 0; i < n; i++) 
  {
	waitpid(tpid_opo[i], &index, 0);
  }
  pthread_join(id_usu, NULL);
  pthread_join(id_temps, NULL);
  
  win_fi();				/* tanca les curses */

  if (*fi1 == -1) 
	printf("S'ha aturat el joc amb tecla RETURN!\n\n");
  else 
  { 
	if ((*fi1 == 1) && (*oponents_morts == n_oponents))
		printf("S'ha produït un empat!                                   \n\n");
	else 
	{
		if (*oponents_morts == n_oponents)
		{
			fprintf(fitxer, "%s %s tron guanyat %c: %d\n",dia,hora,'0',n_usu);
			printf("Ha guanyat l'usuari!                                   \n\n");
		}
		else
			printf("Ha guanyat l'ordinador!                                 \n\n");
	}
  }

  fclose(fitxer);
  elim_mem(id_camp);
  elim_mem(id_fi1);
  elim_mem(id_oponents_morts);
  pthread_mutex_destroy(&mutex);
  elim_sem(id_sem);
  elim_mis(id_bustia_xocs);
  elim_mis(id_bustia_morts);
  free(p_usu);	  	 /* allibera la memoria dinamica obtinguda */
  return(0);
}
