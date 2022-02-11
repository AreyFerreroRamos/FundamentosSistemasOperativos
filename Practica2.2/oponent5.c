/*******************************************************************************/
/*									       */
/*				     oponent5.c				       */
/*									       */
/* Funcio per moure un oponent una posicio; retorna 1 si l'oponent xoca contra */
/* 	alguna cosa, 0 altrament.					       */
/*									       */
/*  Compilar i executar:					  	       */
/*     El programa invoca les funcions definides a "winsuport2.c", les         */
/*     quals proporcionen una interficie senzilla per crear una finestra       */
/*     de text on es poden escriure caracters en posicions especifiques de     */
/*     la pantalla (basada en CURSES); per tant, el programa necessita ser     */
/*     compilat amb la llibreria 'curses':				       */
/*									       */
/*	   $ gcc -c winsuport2.c -o winsuport2.o			       */
/*	   $ gcc oponent5.c winsuport2.o -o oponent5 -lcurses		       */
/*									       */
/*******************************************************************************/


#include <stdio.h>		/* incloure definicions de funcions estandard */		
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include <time.h>
#include "winsuport2.h"		/* incloure definicions de funcions propies */
#include "memoria.h"
#include "semafor.h"
#include "missatge.h"


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

int df[] = {-1, 0, 1, 0};	/* moviments de les 4 direccions possibles */
int dc[] = {0, -1, 0, 1};	/* dalt, esquerra, baix, dreta */



	/* variables globals */
int n_fil, n_col;		/* dimensions del camp de joc */

tron opo;		/* informacio dels oponents */

int varia;		/* valor de variabilitat dels oponents [0..9] */
FILE *fitxer;           /* descriptor de fitxer on escriurem les longituts */
int retard;		/* valor del retard de moviment, en mil.lisegons */

int *fi1;
int *oponents_morts;
char dia[9];
char hora[9];
int fi2;
int invers, mort;

pos *p_opo;		/* els jugadors */
int n_opo = 0;	/* numero d'entrades en les taules de pos. */

int id_sem, id_bustia_xocs, id_bustia_morts;


/* funcio per esborrar totes les posicions anteriors, sigui de l'usuari o */
/* de l'oponent */
void esborrar_posicions(char car_tron, pos p_pos[], int n_pos, void * index)
{
	int i;
	char cars;

	fprintf(fitxer, "%s %s tron acabat %c: %d\n",dia,hora,car_tron,n_pos);
	//fprintf(stderr, "esborrant %d posicions!\n",n_pos);
	for (i=n_pos-1; i>=0; i--)		/* de l'ultima cap a la primera */
	{
		//fprintf(stderr, "esborrant posicio %d: (%d,%d)\n",i,p_pos[i].f,p_pos[i].c);
		waitS(id_sem);
		cars = win_quincar(p_pos[i].f,p_pos[i].c);
		if (cars == ('1' + (intptr_t) index)) {
			win_escricar(p_pos[i].f,p_pos[i].c,' ',NO_INV);	/* esborra una pos. */
		}
		signalS(id_sem);
		win_retard(10);		/* un petit retard per simular el joc real */
	}
}

/* funcio per inicialitar les variables i visualitzar l'estat inicial del joc */
void inicialitza_joc(int num_oponents, int index)
{
  srand(time(NULL) + index);

  opo.f = ((n_fil-1)/(num_oponents+1))*(index+1);
  opo.c = (n_col*3)/4;			/* fixa posicio i direccio inicial oponent */
  opo.d = rand() % 4;
  waitS(id_sem);
  win_escricar(opo.f,opo.c,'1' + index,INVERS);	/* escriu la primer posicio oponent */
  signalS(id_sem);
  p_opo[n_opo].f = opo.f;		/* memoritza posicio inicial */
  p_opo[n_opo].c = opo.c;
  n_opo++;
}

/* funció per obtenir els missatges de la bustia */
void * comprovar_bustia_xocs (void * index)
{
  char missatge[2];

  do
  {
	receiveM(id_bustia_xocs, missatge);
	if (*missatge == ('1' + (intptr_t) index)) {
		invers = 0;
		win_retard(10000);
		invers = 1;
	}
	else
		sendM(id_bustia_xocs, missatge, 1);	
  } while (!(*fi1) && !fi2);
  return ((void *)(intptr_t) 0);	
}

/* funció per obtenir els missatges de la bustia */
void * comprovar_bustia_morts (void * index)
{
 char missatge[2];

  do
  {
	receiveM(id_bustia_morts, missatge);
	if (*missatge == ('1' + (intptr_t) index))
		mort = 1;
	else
		sendM(id_bustia_morts, missatge, 2);
  } while (!(*fi1) && !fi2);
  return ((void *)(intptr_t) 0);
}

/* Programa principal */
int main(int n_args, char *ll_args[])
{
  char cars, missatge[2];
  int atrib;
  tron seg;
  int k, vk, nd, vd[3];
  int canvi, id_camp, *p_camp, id_fi1, id_oponents_morts, index, num_oponents;
  
  pthread_t id_comprovar_bustia_xocs, id_comprovar_bustia_morts;

  if (n_args != 14)
  {
	fprintf(stderr,"Comanda: ./oponent5 variabilitat [retard]\n");
	fprintf(stderr,"	 on \'num_oponents\' indica el nombre d'oponents del joc\n");
  	fprintf(stderr,"         on \'variabilitat\' indica la frequencia de canvi de direccio\n");
  	fprintf(stderr,"         de l'oponent: de 0 a 3 (0- gens variable, 3- molt variable),\n");
  	fprintf(stderr,"         fitxer es el fitxer on acumularem la llargaria dels drons ),\n");
  	fprintf(stderr,"         i \'retard\' es el numero de mil.lisegons que s'espera entre dos\n");
  	exit(1);
  }
  
  id_camp=atoi(ll_args[1]);
  p_camp=map_mem(id_camp);
  n_fil=atoi(ll_args[2]);
  n_col=atoi(ll_args[3]);
  win_set(p_camp, n_fil, n_col);

  varia=atoi(ll_args[4]);
  retard=atoi(ll_args[5]);
  id_fi1=atoi(ll_args[6]);
  fi1=map_mem(id_fi1);
  id_oponents_morts=atoi(ll_args[7]);
  oponents_morts=map_mem(id_oponents_morts);
  index=atoi(ll_args[8]);
  num_oponents=atoi(ll_args[9]);
  
  id_sem=atoi(ll_args[10]);
  id_bustia_xocs=atoi(ll_args[11]);
  id_bustia_morts=atoi(ll_args[12]);
  
  fitxer = fopen(ll_args[13],"a");

  p_opo = calloc(n_fil*n_col/2, sizeof(pos));	/* per a les posicions ant. */
  if (!p_opo)	/* si no hi ha prou memoria per als vectors de pos. */
  { 
	win_fi();				/* tanca les curses */
	fprintf(stderr,"Error en alocatacion de memoria dinamica.\n");
	exit(3);
  }

  inicialitza_joc(num_oponents, index);

  fi2 = 0;
  invers = 1;
  mort = 0;

  pthread_create(&id_comprovar_bustia_xocs, NULL, comprovar_bustia_xocs, (void *)(intptr_t) index);
  pthread_create(&id_comprovar_bustia_morts, NULL, comprovar_bustia_morts, (void *)(intptr_t) index); 

  do 
  {
	canvi = 0;
  	seg.f = opo.f + df[opo.d];	/* calcular seguent posicio */
  	seg.c = opo.c + dc[opo.d];
	waitS(id_sem);
  	cars = win_quincar(seg.f,seg.c);	/* calcula caracter seguent posicio */
	atrib = win_quinatri(seg.f,seg.c);
  	if ((cars != ' ') && (atrib != 0))			/* si seguent posicio ocupada */
     		canvi = 1;		/* anotar que s'ha de produir un canvi de direccio */
  	else {
		if ((atrib == 0) && (cars != ('1' + (intptr_t) index))) 
		{
			sprintf(missatge,"%c",cars);
			sendM(id_bustia_morts,missatge,2);
		}
    		if (varia > 0)	/* si hi ha variabilitat */
    		{ 
			k = rand() % 10;		/* prova un numero aleatori del 0 al 9 */
			if (k < varia) canvi = 1;	/* possible canvi de direccio */
    		}
  	}
  	if (canvi)		/* si s'ha de canviar de direcció */
  	{
		if (cars != ('1' + (intptr_t) index)) {
			sprintf(missatge,"%c",cars);
			sendM(id_bustia_xocs,missatge,2);
		}
    		nd = 0;
    		for (k=-1; k<=1; k++)	/* provar direccio actual i dir. veines */
    		{
			vk = (opo.d + k) % 4;		/* nova direccio */
			if (vk < 0) vk += 4;		/* corregeix negatius */
			seg.f = opo.f + df[vk];		/* calcular posicio en la nova dir.*/
			seg.c = opo.c + dc[vk];
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
  				opo.d = vd[0];			/* li assigna aquesta */
      			else				/* altrament */
    				opo.d = vd[rand() % nd];	/* segueix una dir. aleatoria */
    		}
  	}
  	if ((fi2 == 0) && (mort == 0))		/* si no ha col.lisionat amb res */
  	{
  		opo.f = opo.f + df[opo.d];			/* actualitza posicio */
    		opo.c = opo.c + dc[opo.d];
		if (invers)
			win_escricar(opo.f,opo.c,'1' + index,INVERS);	/* dibuixa bloc oponent */
		else
			win_escricar(opo.f,opo.c,'1' + index,NO_INV);
		signalS(id_sem);
    		p_opo[n_opo].f = opo.f;			/* memoritza posicio actual */
    		p_opo[n_opo].c = opo.c;
    		n_opo++;
  	}
  	else {
		signalS(id_sem);
		esborrar_posicions('1' + index, p_opo, n_opo, (void *)(intptr_t) index);
		waitS(id_sem);
		(*oponents_morts)++;
		signalS(id_sem);
		if (fi2 == 0)
			fi2 = 1;
	}
	win_retard(retard);
  } while (!(*fi1) && !fi2);
  fprintf(fitxer, "%s %s tron guanyat %c: %d\n",dia,hora,'1' + index,n_opo);
  fclose(fitxer);
  free(p_opo);
  return (index);
}
