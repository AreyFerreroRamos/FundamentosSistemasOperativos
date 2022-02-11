/*****************************************************************************/
/*									     */
/*				     tron2.c				     */
/*									     */
/*  Programa corresponent a la pràctica 2.1 de FSO.	                     */
/*     Es tracta del joc del tron: sobre un camp de joc rectangular, es      */
/*     mouen uns objectes que anomenarem 'trons' (amb o tancada). En aquesta */
/*     segona versio del joc, nomes hi ha un tron que controla l'usuari, que */
/*     representarem amb un '0', i diversos trons que controla l'ordinador   */
/*     que es representara amb un número de '1' a '9'. Els trons son una     */ 
/*     especis de 'motos' que quan es mouen deixen rastre (el caracter       */
/*     corresponent). L'usuari pot canviar la direccio de moviment del seu   */
/*     tron amb les tecles: 'w' (adalt), 's' (abaix), 'd' (dreta) i 'a'      */
/*     (esquerra). Els trons que controla l'ordinador es mouran aleatoriament*/
/*     , canviant de direccio aleatoriament segons un parametre del programa */
/*     (veure Arguments). El joc consisteix en que un tron intentara         */
/*     'tancar' als altres trons. El primer tron que xoca contra un obstacle */
/*     (sigui rastre seu o de l'altre tron), esborrara tot el seu rastre i   */
/*     perdra la partida.						     */
/*									     */
/*  Arguments del programa:						     */
/*     El primer argument serà el número de oponents que es volen crear.     */
/*     per controlar la variabilitat del canvi de direccio, s'ha de propor-  */
/*     cionar com a segon argument un numero del 0 al 3, el qual indicara    */
/*     si els canvis s'han de produir molt frequentment (3 es el maxim) o    */
/*     poc frequentment, on 0 indica que nomes canviara per esquivar les     */
/*     parets.								     */
/*     Hi haura un tercer parametre que sera el nom d'un fitxer on guardarem */
/*     les llargaries dels diferents trons.				     */
/*     A mes, es podra afegir un quart argument opcional per indicar el      */
/*     retard de moviment dels diferents trons (en ms);                      */
/*     el valor per defecte d'aquest parametre es 100 (1 decima de segon).   */
/*									     */
/*  Compilar i executar:					  	     */
/*     El programa invoca les funcions definides a "winsuport.c", les        */
/*     quals proporcionen una interficie senzilla per crear una finestra     */
/*     de text on es poden escriure caracters en posicions especifiques de   */
/*     la pantalla (basada en CURSES); per tant, el programa necessita ser   */
/*     compilat amb la llibreria 'curses':				     */
/*									     */
/*	   $ gcc -c winsuport.c -o winsuport.o				     */
/*	   $ gcc tron2.c winsuport.o -o tron2 -lcurses			     */
/*	   $ ./tron2 num_oponents variabilitat fitxer [retard] 		     */
/*									     */
/*  Codis de retorn:						  	     */
/*     El programa retorna algun dels seguents codis al SO:		     */
/*	0  ==>  funcionament normal					     */
/*	1  ==>  numero d'arguments incorrecte 				     */
/*	2  ==>  no s'ha pogut crear el camp de joc (no pot iniciar CURSES)   */
/*	3  ==>  no hi ha prou memoria per crear les estructures dinamiques   */
/*									     */
/*****************************************************************************/

		
#include <stdio.h>		/* incloure definicions de funcions estandard */		
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <stdint.h>
#include "winsuport.h"		/* incloure definicions de funcions propies */

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
pthread_t tid_opo[MAX_OPONENTS];	/* Taula d'indentificadors dels threads dels oponents */
pthread_t id_usu;		/* identificador del thread de l'ususari */

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

int fi1;
int n_oponents, oponents_morts;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;		/* Crea una semàfors globals */



/* funcio per esborrar totes les posicions anteriors, sigui de l'usuari o */
/* de l'oponent */
void esborrar_posicions(char car_tron, pos p_pos[], int n_pos)
{
	int i;
	fprintf(fitxer, "%s %s tron acabat %c: %d\n",dia,hora,car_tron,n_pos);
	//fprintf(stderr, "esborrant %d posicions!\n",n_pos);
	for (i=n_pos-1; i>=0; i--)		/* de l'ultima cap a la primera */
	{
		//fprintf(stderr, "esborrant posicio %d: (%d,%d)\n",i,p_pos[i].f,p_pos[i].c);
		pthread_mutex_lock(&mutex);
		win_escricar(p_pos[i].f,p_pos[i].c,' ',NO_INV);	/* esborra una pos. */
		pthread_mutex_unlock(&mutex);
		win_retard(10);		/* un petit retard per simular el joc real */
	}
}



/* funcio per inicialitar les variables i visualitzar l'estat inicial del joc */
void inicialitza_joc(int num_oponents)
{
  char strin[45];
  int i;

  usu.f = (n_fil-1)/2;
  usu.c = (n_col)/4;		/* fixa posicio i direccio inicial usuari */
  usu.d = 3;
  win_escricar(usu.f,usu.c,'0',INVERS);	/* escriu la primer posicio usuari */
  p_usu[n_usu].f = usu.f;		/* memoritza posicio inicial */
  p_usu[n_usu].c = usu.c;
  n_usu++;

  for (i = 0; i < num_oponents; i++) {
  	opo[i].f = ((n_fil-1)/(num_oponents+1))*(i+1);
  	opo[i].c = (n_col*3)/4;			/* fixa posicio i direccio inicial oponent */
  	opo[i].d = rand() % 4;
  	win_escricar(opo[i].f,opo[i].c,'1'+i,INVERS);	/* escriu la primer posicio oponent */
  }

  sprintf(strin,"Tecles: \'%c\', \'%c\', \'%c\', \'%c\', RETURN-> sortir\n",
		TEC_AMUNT, TEC_AVALL, TEC_DRETA, TEC_ESQUER);
  win_escristr(strin);
}




/* funcio per moure un oponent una posicio; retorna 1 si l'oponent xoca */
/* contra alguna cosa, 0 altrament					*/
void * mou_oponent(void * index)
{
  char cars;
  tron seg;
  int k, vk, nd, vd[3];
  int canvi;
  pos *p_opo;		/* taula de posicions que van recorrent els jugadors */
  int n_opo = 0;	/* numero d'entrades en les taules de pos. */
  int fi2 = 0;

  p_opo = calloc(n_fil*n_col/2, sizeof(pos));	/* per a les posicions ant. */
  if (!p_opo)	/* si no hi ha prou memoria per als vectors de pos. */
  { 
	win_fi();				/* tanca les curses */
	if (p_usu) free(p_usu);
	fprintf(stderr,"Error en alocatacion de memoria dinamica.\n");
	exit(3);
  }

  p_opo[n_opo].f = opo[(intptr_t) index].f;		/* memoritza posicio inicial */
  p_opo[n_opo].c = opo[(intptr_t) index].c;
  n_opo++;
 
  do 
  {
	canvi = 0;
  	seg.f = opo[(intptr_t) index].f + df[opo[(intptr_t) index].d];	/* calcular seguent posicio */
  	seg.c = opo[(intptr_t) index].c + dc[opo[(intptr_t) index].d]; 
	pthread_mutex_lock(&mutex);
	cars = win_quincar(seg.f,seg.c);	/* calcula caracter seguent posicio */
  	if (cars != ' ')			/* si seguent posicio ocupada */	
     		canvi = 1;		/* anotar que s'ha de produir un canvi de direccio */
  	else
    		if (varia > 0)	/* si hi ha variabilitat */
    		{ 
			k = rand() % 10;		/* prova un numero aleatori del 0 al 9 */
			if (k < varia) canvi = 1;	/* possible canvi de direccio */
    		}
  	if (canvi)		/* si s'ha de canviar de direccio */
  	{
    		nd = 0;
    		for (k=-1; k<=1; k++)	/* provar direccio actual i dir. veines */
    		{
			vk = (opo[(intptr_t) index].d + k) % 4;		/* nova direccio */
			if (vk < 0) vk += 4;		/* corregeix negatius */
			seg.f = opo[(intptr_t) index].f + df[vk];		/* calcular posicio en la nova dir.*/
			seg.c = opo[(intptr_t) index].c + dc[vk];
			cars = win_quincar(seg.f,seg.c);	/* calcula caracter seguent posicio */
			if (cars == ' ')
			{
	  			vd[nd] = vk;			/* memoritza com a direccio possible */
	  			nd++;				/* anota una direccio possible mes */
			}
    		}
    		if (nd == 0)			/* si no pot continuar, */
  			fi2 = 1;		/* xoc: ha perdut l'oponent! */
    		else
    		{ 
			if (nd == 1)			/* si nomes pot en una direccio */
  				opo[(intptr_t) index].d = vd[0];			/* li assigna aquesta */
      			else				/* altrament */
    				opo[(intptr_t) index].d = vd[rand() % nd];	/* segueix una dir. aleatoria */
    		}
  	}
  	if (fi2 == 0)		/* si no ha col.lisionat amb res */
  	{
  		opo[(intptr_t) index].f = opo[(intptr_t) index].f + df[opo[(intptr_t) index].d];			/* actualitza posicio */
    		opo[(intptr_t) index].c = opo[(intptr_t) index].c + dc[opo[(intptr_t) index].d];   		
		win_escricar(opo[(intptr_t) index].f,opo[(intptr_t) index].c,'1' + (intptr_t) index,INVERS);	/* dibuixa bloc oponent */
    		pthread_mutex_unlock(&mutex);
		p_opo[n_opo].f = opo[(intptr_t) index].f;			/* memoritza posicio actual */
    		p_opo[n_opo].c = opo[(intptr_t) index].c;
    		n_opo++;
  	}
  	else
	{
		pthread_mutex_unlock(&mutex);
		esborrar_posicions('1' + (intptr_t) index, p_opo, n_opo);
		pthread_mutex_lock(&mutex);
		oponents_morts++;
		pthread_mutex_unlock(&mutex);
	}
	win_retard(retard);
  } while (!fi1 && !fi2);
  fprintf(fitxer, "%s %s tron guanyat %ld: %d\n",dia,hora,'1' + (intptr_t) index,n_opo);
  free(p_opo);	  	 		/* allibera la memoria dinamica obtinguda */
  return ((void *)(intptr_t) index);
}



/* funcio per moure l'usuari una posicio, en funcio de la direccio de   */
/* moviment actual; retorna -1 si s'ha premut RETURN, 1 si ha xocat     */
/* contra alguna cosa, i 0 altrament */
void * mou_usuari(void * nulo)
{
  char cars;
  tron seg;
  int tecla;
  
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
    			case TEC_RETURN:	fi1 = -1; break;
  		}
 	seg.f = usu.f + df[usu.d];	/* calcular seguent posicio */
  	seg.c = usu.c + dc[usu.d];
	pthread_mutex_lock(&mutex);
  	cars = win_quincar(seg.f,seg.c);	/* calcular caracter seguent posicio */
 	if (cars == ' ')			/* si seguent posicio lliure */
  	{
    		usu.f = seg.f; 		/* actualitza posicio */
		usu.c = seg.c;		
    		win_escricar(usu.f,usu.c,'0',INVERS);	/* dibuixa bloc usuari */
    		pthread_mutex_unlock(&mutex);
		p_usu[n_usu].f = usu.f;		/* memoritza posicio actual */
    		p_usu[n_usu].c = usu.c;
    		n_usu++;
  	}
  	else
  	{
		pthread_mutex_unlock(&mutex);
		esborrar_posicions('0', p_usu, n_usu);
		fi1 = 1;
  	}
	win_retard(retard);
  } while (!fi1 && (oponents_morts < n_oponents));
  return ((void *)(intptr_t) 0);
}


/* programa principal				    */
int main(int n_args, const char *ll_args[])
{
  int n, i, retwin, minuts, segons;		/* variables locals */
  char strin[45];

  time_t tiempo = time(0);
  struct tm *tlocal = localtime(&tiempo);
  strftime(dia,9,"%d/%m/%y",tlocal);
  strftime(hora,9,"%H:%M:%S",tlocal);

  srand(getpid());		/* inicialitza numeros aleatoris */

  if (n_args <= 4)
  {	fprintf(stderr,"Comanda: ./tron2 num_oponents variabilitat fitxer [retard]\n");
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
  {	retard = atoi(ll_args[4]);	/* convertir-lo a enter */
  	if (retard < 10) retard = 10;	/* verificar limits */
  	if (retard > 1000) retard = 1000;
  }
  else retard = 100;		/* altrament, fixar retard per defecte */

  printf("Joc del Tron\n\tTecles: \'%c\', \'%c\', \'%c\', \'%c\', RETURN-> sortir\n",
		TEC_AMUNT, TEC_AVALL, TEC_DRETA, TEC_ESQUER);
  printf("prem una tecla per continuar:\n");
  getchar();

  n_fil = 0; n_col = 0;		/* demanarem dimensions de taulell maximes */
  retwin = win_ini(&n_fil,&n_col,'+',INVERS);	/* intenta crear taulell */

  if (retwin < 0)	/* si no pot crear l'entorn de joc amb les curses */
  { fprintf(stderr,"Error en la creacio del taulell de joc:\t");
    switch (retwin)
    {	case -1: fprintf(stderr,"camp de joc ja creat!\n"); break;
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
  inicialitza_joc(n_oponents);
  pthread_mutex_init(&mutex, NULL);		/* Inicialitza el semàfor global */
  
  oponents_morts = 0;
  fi1 = 0;
  n = 0;
  for (i = 0; i < n_oponents; i++) 
  {
	if (pthread_create(&tid_opo[n], NULL, mou_oponent, (void *)(intptr_t) i) == 0) 
	{
		n++;
	}
  }
  pthread_create(&id_usu, NULL, mou_usuari, (void *)(intptr_t) 0);

  minuts=0;
  segons=0;
  do 
  {					// bucle principal
	sprintf(strin,"Temps de joc: %i:%i", minuts, segons);
	pthread_mutex_lock(&mutex);
  	win_escristr(strin);
	pthread_mutex_unlock(&mutex);
	win_retard(1000);		
	segons++;
	if (segons == 60) 
	{
		minuts++;
		segons=0;
	}
  } while (!fi1 && (oponents_morts < n_oponents));
  
  for (i = 0; i < n; i++) 
  {
	pthread_join(tid_opo[i], NULL);
  }
  pthread_join(id_usu, NULL);
  
  win_fi();				/* tanca les curses */
  pthread_mutex_destroy(&mutex);
  

  if (fi1 == -1) 
	printf("S'ha aturat el joc amb tecla RETURN!\n\n");
  else 
  { 
	if ((fi1 == 1) && (oponents_morts == n_oponents))
		printf("S'ha produït un empat!                                 \n\n");
	else 
	{
		if (oponents_morts == n_oponents) 
		{
			fprintf(fitxer, "%s %s tron guanyat %c: %d\n",dia,hora,'0',n_usu);
			printf("Ha guanyat l'usuari!                                   \n\n");
		}	
		else
			printf("Ha guanyat l'ordinador!                                 \n\n");
	}
  }

  fclose(fitxer);
  free(p_usu);			/* allibera la memoria dinamica obtinguda */
  return(0);
}
