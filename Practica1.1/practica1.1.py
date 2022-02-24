#! /usr/bin/env python

# Autor: Arey Ferrero Ramos.
# Data: 24 de febrer del 2022. Versio: 1.
# Descripcio: Donat un fitxer anomenat 'dates.txt' es vol comprovar que el format dels components de cada linia sigui correcte.

import sys, os.path

if len(sys.argv)==2:
	if os.path.isfile(sys.argv[1]):
		messos=("Jan","Ene","Gen","Feb","Mar","Apr","Abr","May","Mai","Jun","Jul","Aug","Ago","Sep","Set","Oct","Nov","Des","Dic")
		dies=(31,31,31,29,31,30,30,31,31,30,31,31,31,30,30,31,30,31,31)
		dates=open(sys.argv[1],'r')
		for data in dates:
			error=""
			mes=data.split()[0]
			dia=int(data.split()[1])
			hora=int(data.split()[2].split(':')[0])
			minuts=int(data.split()[2].split(':')[1])
			segons=int(data.split()[2].split(':')[2])
			if mes not in messos:
				error=error+"El mes es incorrecte. "
			if dia<0 or dia>31:
				error=error+"El dia es incorrecte. "
                        else:
                                pos=0
                                while mes!=messos[pos]:
                                        pos+=1
                                if dia>dies[pos]:
                                        error=error+"El dia es incorrecte. "
			if hora<0 or hora>23:
				error=error+"L'hora es incorrecta. "
			if minuts<0 or minuts>59:
				error=error+"Els minuts son incorrectes. "
			if segons<0 or segons>59:
				error=error+"Els segons son incorrectes. "
			if error:
				print("La linia '"+data.split('\n')[0]+"' no te el format correcte. "+error)
		dates.close()
	else:
		print("El parametre d'entrada ha de ser un fitxer.")
else:
	print("El numero de parametres es incorrecte.")
