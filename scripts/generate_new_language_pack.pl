#!/usr/bin/perl -w

use strict;
use warnings;

my @templates = qw( webcontent/includes/header.txt webcontent/includes/footer.txt );
my %option;
my $count = 0;
my $lang = '';

#########################################
# Collection options from the command line (ie what ./configure sends us)
# and validate
foreach my $opt ( @ARGV ) {
  if ( $opt =~ /^[a-z]{2}$/ ) {
    $lang = $opt;
    $count++;
  }
  else {
    print "Language prefix should be two lower case letters only.\n";
  }
}
die "You should provide one language prefix only, as a parameter." if $count != 1;


my @resource_files = qw( webcontent/language.resource.en i18n/language.resource.en );
opendir( DIR, 'webcontent/includes/local/' );
while( my $FILE = readdir( DIR ) ) {
  if ( $FILE =~ /\.resource\.en/ ) {
    push @resource_files, 'webcontent/includes/local/'.$FILE;
  }
}
closedir( DIR );

# Load the language pack
foreach my $file (@resource_files) {

  my $file_out = $file;
  $file_out =~ s/en$/$lang/;

  die "Target file ($file_out) already exists." if -f $file_out;
  print "Creating $file_out\n";

  open( SOURCE, $file );
  open( TARGET, ">$file_out" );
  while ( my $line = <SOURCE> ) {
    chomp( $line );
    next if $line =~ /^$/ || $line =~ /^#/ || $line =~ /^\/\//;
    if ( $file =~ /includes\/local/ ) {
      # A javascript localise resource file
      my ( $key, $data ) = $line =~ /^(.*) = '(.*)';/;
      print TARGET "$key = 'LOCALISE_ME - $data';\n";
    }
    else {
      # An application resource file
      my ( $key, $data ) = split( /\|/, $line );
      print TARGET "$key|LOCALISE_ME - $data\n";
    }
  }
  close( TARGET );
  close( SOURCE );
}

__END__
