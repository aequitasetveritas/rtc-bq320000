# -*- coding: latin-1 -*-
#Pequeño script para poner en hora el RTC BQ32000. 
#Es necesario que el micro contenga el programa rct.c
import serial
import datetime
import sys, getopt
import time

def convertir_a_BCD(numstr):
        a = list(bytearray(numstr))
        a = [x - 0x30 for x in a]
        b = a[0]<<4 | a[1]
        return b

def main(argv):
        puerto = ''
        try:
                opts, args = getopt.getopt(argv,"hp:",["port="])
        except getopt.GetoptError:
                print 'poner_en_hora_RTC -p <PORT>'
                sys.exit(2)
                
        for opt, arg in opts:
                if opt == '-h':
                        print 'poner_en_hora_RTC -p <PORT>'
                        sys.exit(0)
                elif opt in ("-p", "--port"):
                        puerto = arg
        
        if not opts:
                #No hay parametros
                print 'Uso: poner_en_hora_RTC -p <PORT>'
                sys.exit(0)

        print("Abriendo..."),
        try:
                ser = serial.Serial(puerto, 9600, timeout=5)
        except serial.SerialException:
                print 'ERROR: no se puede abrir el puerto "' + puerto + '"'
                sys.exit(3)
        
        print ser.name + ' OK'
        
        #Cierro y abro BUG
        ser.close()
        ser.open()
        ser.setDTR(False) #RESET del micro en alto
        time.sleep(2)
        
        
        ser.write('\xc1'); #Envio valor de inicio al micro
        x = '\xFF'
        x = ser.read(1) #Espera 5 segundos para recibir una respuesta del micro
        print bytes(x)
        if x == '\xFF':
                print 'No se recibio la señal de inicio desde el microcontrolador'
        if x == '\xc2':
                print("Recibida respuesta del micro...")
                print("Enviando year..."),
                year = str(datetime.datetime.now().year - 2000)
                print convertir_a_BCD(year)
                ser.write(year.decode("hex"))
                x = ser.read(1)
                
                if x == '\xc3':
                        print("OK")
                        print 'Enviando mes, fecha, dia, hora, minutos, segundos...'
                        now = datetime.datetime.now()
                        time.sleep(1)
                       
                        ser.write('\xBB')
                        x=ser.read(14)
                        print x
                        
                        ser.write(str(now.month).zfill(2).decode("hex"))
                        if ser.read(1) != '\xc4':
                                error_en('mes')
                        time.sleep(1)
                        ser.write('\xbb')
                        x=ser.read(14)
                        print x
                        ser.write(str(now.day).zfill(2).decode("hex"))
                        if ser.read(1) != '\xc5':
                                error_en('fecha')
                        time.sleep(1)
                        ser.write(str(int(time.strftime("%w"))+1).zfill(2).decode("hex"))
                        if ser.read(1) != '\xc6':
                                error_en('dia_de_la_semana')
                        ser.write('\xbb')
                        x=ser.read(14)
                        print x
                        time.sleep(1)
                        now = datetime.datetime.now()
                        ser.write(str(now.hour).zfill(2).decode("hex"))
                        if ser.read(1) != '\xc7':
                                error_en('hora')
                        time.sleep(1)
                        now = datetime.datetime.now()
                        ser.write(str(now.minute).zfill(2).decode("hex"))
                        if ser.read(1) != '\xc8':
                                error_en('minutos')

                        time.sleep(1)
                        now = datetime.datetime.now()
                        ser.write(str(now.second).zfill(2).decode("hex"))
                        if ser.read(1) != '\xc9':
                                error_en('segundos')

                        print 'Datos enviados y recibidos correctamente'
                        time.sleep(1)
                        ser.write('\xbb')
                        x=ser.read(14)
                        print x
                        sys.exit()
                else:
                        print("ERROR Datos enviados pero no recibidos")
        else:
                print("ERROR: Se recibio un comando pero no el esperado")


        sys.exit(0)

def error_en(donde):
        print 'ERROR: ' + donde + ' no recibido'
        sys.exit(4)

if __name__ == "__main__":
   main(sys.argv[1:])
