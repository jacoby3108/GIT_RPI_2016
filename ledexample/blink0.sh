#!/bin/bash


for i in  {1..10}

do
   cat $SENS > $LED
   sleep 2
done

