# Efficient Backup

This is the project of the Operating Systems course in Computer Engineering at the University of Minho.
The project consists in creating an efficient system to backup files. 
The main goals are:

* Minimize the space occupied on disk by the files
* Using a client/server architecture, for privacy purposes
* Server running concurrently, in order to achieve high efficiency


## Download and install

To work with the source code, clone this repository:
	
	$ git clone

To install:
	
	$ make installF

Anytime you want to uninstall:

	$ make uninstallF

## Usage

The server starts with:

	$ sobusrv


Then the client can backup or restore using the commands:

	$ sobucli backup file1 file2
	file1: Sucesso.
	file2: Sucesso.
	$_

	$ sobucli restore file1 file2
	file1: Sucesso.
	file2: Sucesso.
	$_

## Authors

* [André Brandão](https://github.com/andrebrandao21)
* [Hugo Gonçalves](https://github.com/Hugainz)
