#!/bin/bash


echo "infinite loops [ hit CTRL+C to stop]"

while :
do

	LUZ=$( cat $SENS )

	if [ $LUZ == 0 ]; then 


		echo "Se recibio un pulso \n" 
                echo "Generado por alarm_mail.sh" | mail  -s "Alarma Recibida" itba.jacoby@gmail.com
		while [ $LUZ == 0 ]
		do

		LUZ=$( cat $SENS )
		echo $LUZ
		done


	fi

        sleep 1

	echo $LUZ


done

