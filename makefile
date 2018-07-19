.PHONY: default

default: so-commons-library conexiones parsi readline coordinador esi instancia planificador

so-commons-library:
	cd ~; git clone https://github.com/sisoputnfrba/so-commons-library; cd so-commons-library; sudo make install

conexiones:
	cd ~; git clone https://github.com/francobatta/conexiones; cd conexiones; sudo make install
	
parsi:
	cd ~; git clone https://github.com/sisoputnfrba/parsi; cd parsi; sudo make install

readline:
	sudo apt-get install libreadline6 libreadline6-dev

coordinador:
	cd Coordinador; make

esi:
	cd ESI; make
	
instancia:
	cd Instancia; make
	
planificador:
	cd Planificador; make

clean:
	sudo rm -rf ~/so-commons-library
	sudo rm -rf ~/conexiones
	sudo rm -rf ~/parsi
	cd Coordinador; make clean
	cd ESI; make clean
	cd Instancia; make clean
	cd Planificador; make clean