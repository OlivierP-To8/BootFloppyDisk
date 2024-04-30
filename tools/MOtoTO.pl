#!/usr/bin/perl

# Script rapide pour convertir un source asm MO en TO
# Auteur : OlivierP-To8
# Avril 2024

my ($file) = @ARGV;
if (not defined $file)
{
	die "Usage: $0 file.asm\n";
}

open (IN, $file) || die "Cannot open file ".$file." for read";
@lines=<IN>;
close IN;


my $swi=0;

foreach $line (@lines)
{
	# Remplacement points d'entrée standard du moniteur
	if ($line =~ s/SWI/;SWI/ig) {$swi=1; next;}
	if ($swi==1)
	{
		$swi=0;
		$line =~ s/FCB.*\$00/JSR \$E82D/ig;
		$line =~ s/FCB.*\$02/JSR \$E803/ig;
		$line =~ s/FCB.*\$0A/JSR \$E806/ig;
		$line =~ s/FCB.*\$0C/JSR \$E809/ig;
		$line =~ s/FCB.*\$0E/JSR \$E80C/ig;
		$line =~ s/FCB.*\$10/JSR \$E80F/ig;
		$line =~ s/FCB.*\$12/JSR \$E833/ig;
		$line =~ s/FCB.*\$14/JSR \$E821/ig;
		$line =~ s/FCB.*\$16/JSR \$E81B/ig;
		$line =~ s/FCB.*\$18/JSR \$E818/ig;
		$line =~ s/FCB.*\$1A/JSR \$E824/ig;
		$line =~ s/FCB.*\$1C/JSR \$E827/ig;
		$line =~ s/FCB.*\$1E/JSR \$E81E/ig;
		$line =~ s/FCB.*\$20/JSR \$E815/ig;
		$line =~ s/FCB.*\$26/JSR \$E82A/ig;
		$line =~ s/FCB.*\$3C/JSR \$EC00/ig;
		$line =~ s/FCB.*\$42/JSR \$E812/ig;
		goto SUIVANT;
	}

	# Modification des couleurs
	# sur TO Fd Fo B V R B V R
	#		 Fd = fond:  0 => pastel, 1 => foncé
	#		 Fo = forme: 0 => pastel, 1 => foncé
	# sur MO Fo B V R Fd B V R
	#		 Fd = fond:  0 => foncé,  1 => pastel
	#		 Fo = forme: 0 => foncé,  1 => pastel

	# Ignore les data
	if ($line =~ /FCB/) {goto SUIVANT;}
	if ($line =~ /FDB/) {goto SUIVANT;}
	if ($line =~ /ADDD/) {goto SUIVANT;}
	if ($line =~ /SUBD/) {goto SUIVANT;}
	if ($line =~ /#SCROFFSET\+/) {goto SUIVANT;}

	# Points d'entrée standard de la ROM Moniteur ($E800-$FFFF)
	if ($line =~ s/\$A7C0/\$E7C3/ig) {goto SUIVANT;}
	if ($line =~ s/\$A([\da-fA-F]{3})/\$E$1/ig) {goto SUIVANT;}

	# Mémoire écran ($4000 à $5FFF)
	if ($line =~ s/\$0([\da-fA-F]{3})/\$4$1/ig) {goto SUIVANT;}
	if ($line =~ s/\$1([\da-fA-F]{3})/\$5$1/ig) {goto SUIVANT;}

	# Registres du moniteur ($6000 à $60FF)
	#if ($line =~ s/\$2019/\$6019/ig) {goto SUIVANT;} # STATUS
	#if ($line =~ s/\$201A/\$601A/ig) {goto SUIVANT;} # TABPT
	#if ($line =~ s/\$201B/\$601B/ig) {goto SUIVANT;} # RANG
	if ($line =~ s/\$201D/\$601C/ig) {goto SUIVANT;} # TOPTAB
	if ($line =~ s/\$201E/\$601D/ig) {goto SUIVANT;} # TOPRAN
	if ($line =~ s/\$201F/\$601E/ig) {goto SUIVANT;} # BOTTAB
	if ($line =~ s/\$2020/\$601F/ig) {goto SUIVANT;} # BOTRAN
	if ($line =~ s/\$201C/\$6020/ig) {goto SUIVANT;} # COLN
	if ($line =~ s/\$2064/\$6021/ig) {goto SUIVANT;} # IRQPT
	if ($line =~ s/\$2065/\$6022/ig) {goto SUIVANT;} # IRQPT
	if ($line =~ s/\$2067/\$6023/ig) {goto SUIVANT;} # FIRQPT
	if ($line =~ s/\$2068/\$6024/ig) {goto SUIVANT;} # FIRQPT
	if ($line =~ s/\$2061/\$6027/ig) {goto SUIVANT;} # TIMEPT
	if ($line =~ s/\$2062/\$6028/ig) {goto SUIVANT;} # TIMEPT
	if ($line =~ s/\$2070/\$602D/ig) {goto SUIVANT;} # USERAF
	if ($line =~ s/\$2071/\$602E/ig) {goto SUIVANT;} # USERAF
	if ($line =~ s/\$2039/\$6031/ig) {goto SUIVANT;} # TEMPO
	if ($line =~ s/\$203A/\$6032/ig) {goto SUIVANT;} # TEMPO
	if ($line =~ s/\$203B/\$6033/ig) {goto SUIVANT;} # DUREE
	if ($line =~ s/\$203C/\$6034/ig) {goto SUIVANT;} # DUREE
	if ($line =~ s/\$203D/\$6035/ig) {goto SUIVANT;} # TIMBRE
	if ($line =~ s/\$203E/\$6036/ig) {goto SUIVANT;} # OCTAVE
	if ($line =~ s/\$203F/\$6037/ig) {goto SUIVANT;} # OCTAVE
	if ($line =~ s/\$2029/\$6038/ig) {goto SUIVANT;} # FORME
	if ($line =~ s/\$202A/\$6039/ig) {goto SUIVANT;} # ATRANG
	if ($line =~ s/\$202B/\$603B/ig) {goto SUIVANT;} # COLOUR
	if ($line =~ s/\$2032/\$603D/ig) {goto SUIVANT;} # PLOTX
	if ($line =~ s/\$2033/\$603E/ig) {goto SUIVANT;} # PLOTX
	if ($line =~ s/\$2034/\$603F/ig) {goto SUIVANT;} # PLOTY
	if ($line =~ s/\$2035/\$6040/ig) {goto SUIVANT;} # PLOTY
	if ($line =~ s/\$2036/\$6041/ig) {goto SUIVANT;} # CHDRAW
	if ($line =~ s/\$202E/\$6042/ig) {goto SUIVANT;} # CURSFL
	if ($line =~ s/\$202F/\$6043/ig) {goto SUIVANT;} # COPCHR
	#if ($line =~ s/\$2048/\$6048/ig) {goto SUIVANT;} # DKOPC
	#if ($line =~ s/\$2049/\$6049/ig) {goto SUIVANT;} # DKDRV
	#if ($line =~ s/\$204A/\$604A/ig) {goto SUIVANT;} # DKTRK
	#if ($line =~ s/\$204B/\$604B/ig) {goto SUIVANT;} # DKTRK
	#if ($line =~ s/\$204C/\$604C/ig) {goto SUIVANT;} # DKSEC
	#if ($line =~ s/\$204D/\$604D/ig) {goto SUIVANT;} # DKNUM
	#if ($line =~ s/\$204E/\$604E/ig) {goto SUIVANT;} # DKSTA
	#if ($line =~ s/\$204F/\$604F/ig) {goto SUIVANT;} # DKBUF
	#if ($line =~ s/\$2050/\$6050/ig) {goto SUIVANT;} # DKBUF
	#if ($line =~ s/\$2051/\$6051/ig) {goto SUIVANT;} # TRACK0
	#if ($line =~ s/\$2052/\$6052/ig) {goto SUIVANT;} # TRACK0
	#if ($line =~ s/\$2053/\$6053/ig) {goto SUIVANT;} # TRACK1
	#if ($line =~ s/\$2054/\$6054/ig) {goto SUIVANT;} # TRACK1
	#if ($line =~ s/\$2059/\$6059/ig) {goto SUIVANT;} # SEQUCE
	if ($line =~ s/\$2021/\$605A/ig) {goto SUIVANT;} # SCRPT
	if ($line =~ s/\$2022/\$605B/ig) {goto SUIVANT;} # SCRPT
	if ($line =~ s/\$2077/\$605F/ig) {goto SUIVANT;} # SCRMOD
	if ($line =~ s/\$2023/\$6060/ig) {goto SUIVANT;} # STADR
	if ($line =~ s/\$2024/\$6061/ig) {goto SUIVANT;} # STADR
	if ($line =~ s/\$2025/\$6062/ig) {goto SUIVANT;} # ENDDR
	if ($line =~ s/\$2026/\$6063/ig) {goto SUIVANT;} # ENDDR
	if ($line =~ s/\$205A/\$606A/ig) {goto SUIVANT;} # US1
	if ($line =~ s/\$2044/\$606C/ig) {goto SUIVANT;} # TEMP
	if ($line =~ s/\$2045/\$606D/ig) {goto SUIVANT;} # TEMP
	if ($line =~ s/\$2046/\$606E/ig) {goto SUIVANT;} # SAVEST
	if ($line =~ s/\$2047/\$606F/ig) {goto SUIVANT;} # SAVEST
	if ($line =~ s/\$205B/\$6070/ig) {goto SUIVANT;} # ACCENT
	if ($line =~ s/\$205C/\$6071/ig) {goto SUIVANT;} # SS2GET
	if ($line =~ s/\$205D/\$6072/ig) {goto SUIVANT;} # SS3GET
	if ($line =~ s/\$2079/\$6074/ig) {goto SUIVANT;} # CONFIG
	if ($line =~ s/\$2030/\$6075/ig) {goto SUIVANT;} # EFCMPT
	if ($line =~ s/\$2027/\$6076/ig) {goto SUIVANT;} # BLOCZ
	if ($line =~ s/\$2028/\$6077/ig) {goto SUIVANT;} # BLOCZ
	if ($line =~ s/\$202D/\$6078/ig) {goto SUIVANT;} # SCROLS
	#if ($line =~ s/\$2080/\$6080/ig) {goto SUIVANT;} # DKFLG
	#if ($line =~ s/\$20CC/\$60CC/ig) {goto SUIVANT;} # STACK
	if ($line =~ s/\$2078/\$60D2/ig) {goto SUIVANT;} # DECALG
	#if ($line =~ s/\$20FE/\$60FE/ig) {goto SUIVANT;} # TSTRST
	#if ($line =~ s/\$20FF/\$60FF/ig) {goto SUIVANT;} # TSTRST

	# Mémoire utilisateur ($6100 à $9FFF)
	if ($line =~ s/\$2([\da-fA-F]{3})/\$6$1/ig) {goto SUIVANT;}
	if ($line =~ s/\$3([\da-fA-F]{3})/\$7$1/ig) {goto SUIVANT;}
	if ($line =~ s/\$4([\da-fA-F]{3})/\$8$1/ig) {goto SUIVANT;}
	if ($line =~ s/\$5([\da-fA-F]{3})/\$9$1/ig) {goto SUIVANT;}
	# Mémoire utilisateur par banques ($A000 à $DFFF)
	if ($line =~ s/\$6([\da-fA-F]{3})/\$A$1/ig) {goto SUIVANT;}
	if ($line =~ s/\$7([\da-fA-F]{3})/\$B$1/ig) {goto SUIVANT;}
	if ($line =~ s/\$8([\da-fA-F]{3})/\$C$1/ig) {goto SUIVANT;}
	if ($line =~ s/\$9([\da-fA-F]{3})/\$D$1/ig) {goto SUIVANT;}

SUIVANT:
	print $line;
}
