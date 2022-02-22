# !/bin/bash

# Autor: Arey Ferrero Ramos.
# Data: 20 de febrer del 2022. Versió: 2. 
# Descripció : Donat un fitxer anomenat 'dates.txt' es vol comprovar que el format dels components de cada línia sigui correcte.

if [ $# -eq 1 ]
then
	if [ -f $1 ]
	then
		IFS=$'\n'
		for linia in $(cat $1)
		do
			error=""
			mes=$(echo $linia | tr -s ' ' | cut -f1 -d' ')
			dia=$(echo $linia | tr -s ' ' | cut -f2 -d' ')
			hora=$(echo $linia | tr -s ' ' | cut -f3 -d' ' | cut -f1 -d':')
			minuts=$(echo $linia | tr -s ' ' | cut -f3 -d' ' | cut -f2 -d':')
			segons=$(echo $linia | tr -s ' ' | cut -f3 -d' ' | cut -f3 -d':')
			if [ $mes != "Jan" ] && [ $mes != "Ene" ] && [ $mes != "Gen" ] && [ $mes != "Feb" ] && [ $mes != "Mar" ] && [ $mes != "Apr" ] && [ $mes != "Abr" ] && [ $mes != "May" ] && 
				[ $mes != "Mai" ] && [ $mes != "Jun" ] && [ $mes != "Jul" ] && [ $mes != "Aug" ] && [ $mes != "Ago" ] && [ $mes != "Sep" ] && [ $mes != "Set" ] && [ $mes != "Oct" ] &&
				[ $mes != "Nov" ] && [ $mes != "Des" ] && [ $mes != "Dic" ]
			then
				error=" El mes és incorrecte."
			fi
			if [ $dia -lt 1 ] || [ $dia -gt 31 ] 
			then 
				error="$error El dia és incorrecte."
			elif [ $mes = "Apr" ] || [ $mes = "Abr" ] || [ $mes = "Jun" ] || [ $mes = "Sep" ] || [ $mes = "Set" ] || [ $mes = "Nov" ]
			then
				if [ $dia -gt 30 ]
				then	
					error="$error El dia és incorrecte." 
				fi
			elif [ $mes = "Feb" ] && [ $dia -gt 29 ]
			then
				error="$error El dia és incorrecte."
			fi
			if [ $hora -lt 0 ] || [ $hora -gt 23 ]
			then
				error="$error L'hora és incorrecta."
			fi
			if [ $minuts -lt 0 ] || [ $minuts -gt 59 ]
			then
				error="$error Els minuts són incorrectes."
			fi
			if [ $segons -lt 0 ] || [ $segons -gt 59 ]
			then
				error="$error Els segons són incorrectes."
			fi
			if [ ! -z $error ]
			then
				echo -e "La línia '$linia' no té el format correcte:$error"
			fi
		done
		exit 0
	else
		echo -e "El paràmetre d'entrada ha de ser un fitxer." >&2
		exit 1
	fi
else
	echo -e "Us: $0 dates.txt" >&2
	exit 2
fi
