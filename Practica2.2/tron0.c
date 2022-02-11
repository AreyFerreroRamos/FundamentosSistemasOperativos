/*****************************************************************************/
/*									     */
/*				     tron0.c				     */
/*									     */
/*  Programa inicial d'exemple per a les practiques 2 i 3 de FSO.	     */
/*     Es tracta del joc del tron: sobre un camp de joc rectangular, es      */
/*     mouen uns objectes que anomenarem 'trons' (amb o tancada). En aquesta */
/*     primera versio del joc, nomes hi ha un tron que controla l'usuari, i  */
/*     que representarem amb un '0', i un tron que controla l'ordinador, el  */
/*     qual es representara amb un '1'. Els trons son una especis de 'motos' */
/*     que quan es mouen deixen rastre (el caracter corresponent). L'usuari  */
/*     pot canviar la direccio de moviment del seu tron amb les tecles:      */
/*     'w' (adalt), 's' (abaix), 'd' (dreta) i 'a' (esquerra). El tron que   */
/*     controla l'ordinador es moura aleatoriament, canviant de direccio     */
/*     aleatoriament segons un parametre del programa (veure Arguments).     */
/*     El joc consisteix en que un tron intentara 'tancar' a l'altre tron.   */
/*     El primer tron que xoca contra un obstacle (sigui rastre seu o de     */
/*     l'altre tron), esborrara tot el seu rastre i perdra la partida.       */
/*									     */
/*  Arguments del programa:						     */
/*     per controlar la variabilitat del canvi de direccio, s'ha de propor-  */
/*     cionar com a primer argument un numero del 0 al 3, el qual indicara   */
/*     si els canvis s'han de produir molt frequentment (3 es el maxim) o    */
/*     poc frequentment, on 0 indica que nomes canviara per esquivar les     */
/*     parets.								     */
/*     Hi haura un segon parametre que sera el nom d'un fitxer on guardarem  */
/*     les llargaries dels diferents trons.				     */
/*     A mes, es podra afegir un tercer argument opcional per indicar el     */
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
/*	   $ gcc tron0.c winsuport.o -o tron0 -lcurses			     */
/*	   $ ./tron0 variabilitat fitxer [retard]			     */
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
#include "winsuport.h"		/* incloure definicions de funcions propies */


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
int n_fil, n_col;		/* dimensions del camp de joc */

tron usu;   	   		/* informacio de l'usuari */
tron opo;			/* informacio de l'oponent */

int df[] = {-1, 0, 1, 0};	/* moviments de les 4 direccions possibles */
int dc[] = {0, -1, 0, 1};	/* dalt, esquerra, baix, dreta */

int varia;		/* valor de variabilitat dels oponents [0..9] */
FILE *fitxer;           /* descriptor de fitxer on escriurem les longituts */
int retard;		/* valor del retard de moviment, en mil.lisegons */

pos *p_usu;			/* taula de posicions que van recorrent */
pos *p_opo;			/* els jugadors */
int n_usu = 0, n_opo = 0;	/* numero d'entrades en les taules de pos. */
char dia[9];
char hora[9];


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
    win_escricar(p_pos[i].f,p_pos[i].c,' ',NO_INV);	/* esborra una pos. */
    win_retard(10);		/* un petit retard per simular el joc real */
  }
}



/* funcio per inicialitar les variables i visualitzar l'estat inicial del joc */
void inicialitza_joc(void)
{
  char strin[45];

  usu.f = (n_fil-1)/2;
  usu.c = (n_col)/4;		/* fixa posicio i direccio inicial usuari */
  usu.d = 3;
  win_escricar(usu.f,usu.c,'0',INVERS);	/* escriu la primer posicio usuari */
  p_usu[n_usu].f = usu.f;		/* memoritza posicio inicial */
  p_usu[n_usu].c = usu.c;
  n_usu++;

  opo.f = (n_fil-1)/2;
  opo.c = (n_col*3)/4;		/* fixa posicio i direccio inicial oponent */
  opo.d = 1;
  win_escricar(opo.f,opo.c,'1',INVERS);	/* escriu la primer posicio oponent */
  p_opo[n_opo].f = opo.f;		/* memoritza posicio inicial */
  p_opo[n_opo].c = opo.c;
  n_opo++;

  sprintf(strin,"Tecles: \'%c\', \'%c\', \'%c\', \'%c\', RETURN-> sortir\n",
		TEC_AMUNT, TEC_AVALL, TEC_DRETA, TEC_ESQUER);
  win_escristr(strin);
}




/* funcio per moure un oponent una posicio; retorna 1 si l'oponent xoca */
/* contra alguna cosa, 0 altrament					*/
int mou_oponent(void)
{
  char cars;
  tron seg;
  int k, vk, nd, vd[3];
  int canvi = 0;
  int retorn = 0;
 
  seg.f = opo.f + df[opo.d];	/* calcular seguent posicio */
  seg.c = opo.c + dc[opo.d];
  cars = win_quincar(seg.f,seg.c);	/* calcula caracter seguent posicio */
  if (cars != ' ')			/* si seguent posicio ocupada */
     canvi = 1;		/* anotar que s'ha de produir un canvi de direccio */
  else
    if (varia > 0)	/* si hi ha variabilitat */
    { k = rand() % 10;		/* prova un numero aleatori del 0 al 9 */
      if (k < varia) canvi = 1;	/* possible canvi de direccio */
    }
  
  if (canvi)		/* si s'ha de canviar de direccio */
  {
    nd = 0;
    for (k=-1; k<=1; k++)	/* provar direccio actual i dir. veines */
    {
	vk = (opo.d + k) % 4;		/* nova direccio */
	if (vk < 0) vk += 4;		/* corregeix negatius */
	seg.f = opo.f + df[vk];		/* calcular posicio en la nova dir.*/
	seg.c = opo.c + dc[vk];
	cars = win_quincar(seg.f,seg.c);/* calcula caracter seguent posicio */
	if (cars == ' ')
	{ vd[nd] = vk;			/* memoritza com a direccio possible */
	  nd++;				/* anota una direccio possible mes */
	}
    }
    if (nd == 0)			/* si no pot continuar, */
  	retorn = 1;		/* xoc: ha perdut l'oponent! */
    else
    { if (nd == 1)			/* si nomes pot en una direccio */
  	opo.d = vd[0];			/* li assigna aquesta */
      else				/* altrament */
    	opo.d = vd[rand() % nd];	/* segueix una dir. aleatoria */
    }
  }
  if (retorn == 0)		/* si no ha col.lisionat amb res */
  {
    opo.f = opo.f + df[opo.d];			/* actualitza posicio */
    opo.c = opo.c + dc[opo.d];
    win_escricar(opo.f,opo.c,'1',INVERS);	/* dibuixa bloc oponent */
    p_opo[n_opo].f = opo.f;			/* memoritza posicio actual */
    p_opo[n_opo].c = opo.c;
    n_opo++;
  }
  else esborrar_posicions('1', p_opo, n_opo);

  return(retorn);
}



/* funcio per moure l'usuari una posicio, en funcio de la direccio de   */
/* moviment actual; retorna -1 si s'ha premut RETURN, 1 si ha xocat     */
/* contra alguna cosa, i 0 altrament */
int mou_usuari(void)
{
  char cars;
  tron seg;
  int tecla, retorn;
  
  retorn = 0;
  tecla = win_gettec();
  if (tecla != 0)
   switch (tecla)	/* modificar direccio usuari segons tecla */
   {
    case TEC_AMUNT:	usu.d = 0; break;
    case TEC_ESQUER:	usu.d = 1; break;
    case TEC_AVALL:	usu.d = 2; break;
    case TEC_DRETA:	usu.d = 3; break;
    case TEC_RETURN:	retorn = -1; break;
   }
  seg.f = usu.f + df[usu.d];	/* calcular seguent posicio */
  seg.c = usu.c + dc[usu.d];
  cars = win_quincar(seg.f,seg.c);	/* calcular caracter seguent posicio */
  if (cars == ' ')			/* si seguent posicio lliure */
  {
    usu.f = seg.f; usu.c = seg.c;		/* actualitza posicio */
    win_escricar(usu.f,usu.c,'0',INVERS);	/* dibuixa bloc usuari */
    p_usu[n_usu].f = usu.f;		/* memoritza posicio actual */
    p_usu[n_usu].c = usu.c;
    n_usu++;
  }
  else
  { esborrar_posicions('0', p_usu, n_usu);
    retorn = 1;
  }
  return(retorn);
}


/* programa principal				    */
int main(int n_args, const char *ll_args[])
{
  int fi1, fi2, retwin;		/* variables locals */

  time_t tiempo = time(0);
  struct tm *tlocal = localtime(&tiempo);
  strftime(dia,9,"%d/%m/%y",tlocal);
  strftime(hora,9,"%H:%M:%S",tlocal);

  srand(getpid());		/* inicialitza numeros aleatoris */

  if ((n_args <=  3))
  {	fprintf(stderr,"Comanda: ./tron0 variabilitat fitxer [retard]\n");
  	fprintf(stderr,"         on \'variabilitat\' indica la frequencia de canvi de direccio\n");
  	fprintf(stderr,"         de l'oponent: de 0 a 3 (0- gens variable, 3- molt variable),\n");
  	fprintf(stderr,"         fitxer es el fitxer on acumularem la llargaria dels drons ),\n");
  	fprintf(stderr,"         i \'retard\' es el numero de mil.lisegons que s'espera entre dos\n");
  	exit(1);
  }
  varia = atoi(ll_args[1]);	/* obtenir parametre de variabilitat */
  if (varia < 0) varia = 0;	/* verificar limits */
  if (varia > 3) varia = 3;

  fitxer = fopen(ll_args[2],"a");
  if (n_args == 3)		/* si s'ha especificat parametre de retard */
  {	retard = atoi(ll_args[3]);	/* convertir-lo a enter */
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
  p_opo = calloc(n_fil*n_col/2, sizeof(pos));	/* per a les posicions ant. */
  if (!p_usu || !p_opo)	/* si no hi ha prou memoria per als vectors de pos. */
  { win_fi();				/* tanca les curses */
    if (p_usu) free(p_usu);
    if (p_opo) free(p_opo);	   /* allibera el que hagi pogut obtenir */
    fprintf(stderr,"Error en alocatacion de memoria dinamica.\n");
    exit(3);
  }
			/* Fins aqui tot ha anat be! */
  inicialitza_joc();
  fi1 = 0;
  fi2 = 0;
  do			/********** bucle principal del joc **********/
  {
	fi1 = mou_usuari();
	fi2 = mou_oponent();
	win_retard(retard);
  } while (!fi1 && !fi2);
  
  win_fi();				/* tanca les curses */

  if (fi1 == -1) printf("S'ha aturat el joc amb tecla RETURN!\n\n");
  else { 
    if (fi2) {
       fprintf(fitxer, "%s %s tron guanyat %c: %d\n",dia,hora,'0',n_usu);
       printf("Ha guanyat l'usuari!                                   \n\n");
    }
    else {
       fprintf(fitxer, "%s %s tron guanyat %c: %d\n",dia,hora,'1',n_opo);
       printf("Ha guanyat l'ordinador!                                 \n\n"); 
    }
  }

  fclose(fitxer);
  free(p_usu);
  free(p_opo);	  	 /* allibera la memoria dinamica obtinguda */
  return(0);
}
