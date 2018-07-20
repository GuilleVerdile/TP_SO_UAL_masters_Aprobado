first: uninstallCommons clean

second: so-commons-library conexiones parsi readline

third: coordinador esi instancia planificador

uninstallCommons:
	cd ~; sudo rm -rf ~/so-commons-library; git clone https://github.com/sisoputnfrba/so-commons-library.git; cd so-commons-library; sudo make uninstall
	cd ~; sudo rm -rf ~/conexiones; git clone https://github.com/francobatta/conexiones.git; cd conexiones; sudo make uninstall
	cd ~; sudo rm -rf ~/parsi; git clone https://github.com/sisoputnfrba/parsi.git

so-commons-library:
	cd ~; cd so-commons-library; sudo make install

conexiones:
	cd ~; cd conexiones; sudo make install
	
parsi:
	cd ~; cd parsi; sudo make install

readline:
	sudo apt-get install libreadline6 libreadline6-dev

coordinador:
	cd ../Coordinador; sudo make all

esi:
	cd ../ESI; sudo make all
	
instancia:
	cd ../Instancia; sudo make all
	
planificador:
	cd ../Planificador; sudo make all

clean:
	cd ../Coordinador; rm -rf  ./Coordinador.o  ./Coordinador.d  Coordinador
	cd ../ESI; rm -rf  ./ESI.o  ./ESI.d  ESI
	cd ../Instancia; rm -rf  ./Instancia.o  ./Instancia.d  Instancia
	cd ../Planificador; rm -rf  ./Consola.o ./Planificador.o ./Colores.o ./Consola.d ./Planificador.d ./Colores.d Planificador