#!/usr/bin/perl -w
use strict ;

my $DEFAULT_ZONE_TAB = "/usr/share/zoneinfo/zone.tab" ;

usage(), exit 1 unless @ARGV ;

main() ;

sub main
{
  # 0. Read the mcc list from file given in --mcc-main=.....
  my $MCC = read_mcc_list_by_parameter("mcc-main") ;

  # 1. Find all the wiki page on the command line and take the latest one
  my $wiki = read_wiki() ;

  # 2. Find and open zone.tab file on the command line: --olson=.......
  my ($olson_tab, $full_list) = read_zonetab_by_parameter("olson", "$DEFAULT_ZONE_TAB") ;
  # ... and --single
  my ($single_tab) = read_zonetab_by_parameter("single") ;
  # ... and --distinct
  my ($distinct_tab) = read_zonetab_by_parameter("distinct") ;

  # 3. Process tiny countries
  my $single1 = find_countries(1, $MCC, $wiki, $olson_tab) ;
  my $single2 = find_countries(1, $MCC, $wiki, $single_tab) ;
  my @single = (sort { $a->{tz} cmp $b->{tz} } @$single1, @$single2) ;

  # 4. Process countries with clear defined timezones (not like US): --distinct=...
  my $distinct = find_countries(0, $MCC, $wiki, $distinct_tab) ;

  # $1. Output tables
  output_single(\@single, "single-output") ;
  output_distinct($distinct, "distinct-output") ;
  output_full($full_list, "full-output") ;

  # $$. Check unused flags
  usage() and die "Invalid command line parameters: " . join(", ", @ARGV) . "\n" if @ARGV ;

  # $$. Check unhandled codes
  my @uhoh = grep { not defined $MCC->{$_} } keys %$MCC ;
  die "Unhandled codes: " . join(", ", sort @uhoh) . ".\n"  if @uhoh ;

  exit 0 ;
}

# 0
sub read_mcc_list_by_parameter
{
  my $flag = shift ;
  my $file = extract_single_parameter($flag) ;
  print "Reading mcc codes from $file... " ;
  open FILE, "<", $file or die "can't read '$file': $!" ;
  my $list = { } ;
  while(<FILE>)
  {
    chomp ;
    next if /^\s*($|#)/ ;
    die "mcc file '$file' is corrupted" unless /^\d{3}$/ ;
    die "mcc file '$file' contains duplicate value $_" if exists $list->{$_} ;
    $list->{$_} = undef ;
  }
  print scalar keys %$list, " codes\n" ;
  return $list ;
}

# 1
sub read_wiki
{
  my $re = qr/^mcc-wikipedia-\d{4}-\d{2}-\d{2}(-*)?\.html$/ ;
  my @wiki_list = sort grep { /$re/ } @ARGV ;
  die "not a single wikipage given on command line" unless @wiki_list ;
  my $wiki_file = $wiki_list[-1] ;
  @ARGV = grep { ! /$re/ } @ARGV ;
  print "Using wikipedia file $wiki_file\n" ;
  open WIKI, "<", "$wiki_file" or die "$wiki_file: @!" ;
  my $wiki = { } ;
  while(<WIKI>)
  {
    chomp ;
    #| 412 || AF || [[Afghanistan]]
    next unless /^\|\s+(\d{3})\s+\|\|\s+([A-Z]{2})\s+\|\|\s*(.*)$/ ;
    my ($mcc, $xy, $comment) = ($1,$2,$3) ;
    print STDERR "Warning: duplicate mcc=$mcc (country=$xy) ignored\n" and next if exists $wiki->{$mcc} ;
    $wiki->{$mcc} = { mcc=>$1, xy=>$2, comment=>$3 } ;
  }
  my $size = scalar keys %$wiki ;
  print "$size mappings mcc=>country in the wiki file\n" ;
  return $wiki ;
}

# 2
sub read_zonetab_by_parameter
{
  my ($flag, $default) = (shift,shift) ;
  my $zone_tab_file = extract_single_parameter($flag, $default) ;
  $zone_tab_file = $default unless $zone_tab_file ;
  print "Reading zone table from $zone_tab_file... " ;
  open ZONES, "<", "$zone_tab_file" or die "$zone_tab_file: $!" ;
  my $zones = { } ;
  my $full = [] ;
  while(<ZONES>)
  {
    chomp ;
    next if /^\s*(#|$)/ ; # comment or empty line
    my @xxx = split(/\s+/, $_, 4) ;
    print STDERR "Warning: invalid line (splitted in ", scalar @xxx, "): $_\n" and next if @xxx < 3 ;
    # xxx = [ "XY", "-45.2343,+23.3049", "Omerigo/Metropolis", "blah blah" ]
    my ($xy, $comment, $tz, $comm2) = @xxx ;
    $comment .= " " . $comm2 if $comm2 ;
    $zones->{$xy} = [ ] unless exists $zones->{$xy} ;
    my $z_xy = $zones->{$xy} ;
    push @$z_xy, { tz=>$tz, comment=>$comment } ;
    push @$full, { tz=>$tz, comment=>"($xy) $comment" } ;
  }
  my $size = scalar keys %$zones ;
  print "$size time zones\n" ;
  return ($zones, $full) ;
}

# 3
sub find_countries
{
  my ($tiny, $MCC, $wiki, $zonetab) = (shift,shift,shift,shift) ;
  my $res = [ ] ;
  foreach my $mcc (sort keys %$MCC)
  {
    next unless exists $wiki->{$mcc} ; # Unknown geographical location
    my $xy = $wiki->{$mcc}->{xy} ;
    next unless exists $zonetab->{$xy} ; # No country in the table
    my $wiki_comment = $wiki->{$mcc}->{comment} ;
    my $zones = $zonetab->{$xy} ;
    if(@$zones == 1 and $tiny==1) # A single timezone
    {
      my $zone = $zones->[0] ;
      my $tz = $zone->{tz} ;
      my $olson_comment = $zone->{comment} ;
      my $full_comment = "($xy) $olson_comment $wiki_comment" ;
      my $entry = { mcc=>$mcc, tz=>$tz, comment=>$full_comment } ;
      push @$res, $entry ;
      die "Fatal: MCC=$mcc alreasy processed!\n" if defined $MCC->{$mcc} ;
      $MCC->{$mcc} = "single" ;
    }
    elsif($tiny==0) # Many distinct timzones
    {
      print STDERR "Warning: a single timezone in a large country: MCC=$mcc ($xy)\n" and next if @$zones == 1 ;
      print STDERR "Warning: MCC=$mcc alreasy processed!\n" and next if defined $MCC->{$mcc} ;
      my $array = [] ;
      push @$array, $_->{tz} foreach (@$zones) ;
      my $full_comment = $xy . " " . $wiki_comment ;
      my $entry = { mcc=>$mcc, xy=>$xy, tz=>$array, comment=>$full_comment } ;
      push @$res, $entry ;
      $MCC->{$mcc} = "distinct" ;
    }
  }
  my $size = scalar @$res ;
  print "$size mcc codes processed " ;
  print $tiny ? "(single timezone)" : "(distinct timezones)", "\n" ;
  return $res ;
}

# $1

# XXX The list is huge, maybe it's better to print just two arrays
#     That whould be less readable for humans,
#     but much more faster to read for stupid computer. Let's see...
sub output_single
{
  my ($map, $flag) = (shift,shift) ;
  my $file = extract_single_parameter($flag, 1) ;
  return unless $file ;
  open FILE, ">", "$file" or die "can't write to $file: $!" ;
  print FILE "# NEVER TOUCH THIS GENERATED FILE\n" ;
  print FILE "list = [\n" ;
  my $q = '"' ;
  for my $i (@$map)
  {
    my $comma = "," ;
    $comma = "" if $i eq $map->[-1] ;
    print FILE "  { mcc = ", sprintf("%3d", $i->{mcc}), ", tz = $q", $i->{tz}, "$q }$comma // ", $i->{comment}, "\n" ;
  }
  print FILE "] .\n" ;
  close FILE or die "can't write to $file: $!" ;
  print "List of single zone countries (", scalar @$map, " mcc) is written to '$file'\n" ;
}

# XXX: replace all the "simple" words by "distinct"
sub output_distinct
{
  my ($map, $flag) = (shift,shift) ;
  my $file = extract_single_parameter($flag, 1) ;
  return unless $file ;
  open FILE, ">", "$file" or die "can't write to $file: $!" ;

  my $q = '"' ;
  my $txt = join (",\n",
    map {
      "  # " .  $_->{comment} . "\n" .
      "  { mcc = " . sprintf("%3d", $_->{mcc}) . ", " .
      "tz = [" .  join(", ", map { "$q$_$q" } @{$_->{tz}}) . "] }"
    } @$map
  ) ;

  print FILE "# NEVER TOUCH THIS GENERATED FILE\n" ;
  print FILE "list = [\n$txt\n] .\n" ;
  close FILE or die "can't write to $file: $!" ;
  print "List of countries with distinct zones (", scalar @$map, " mcc) is written to '$file'\n" ;
}

sub output_full
{
  my ($list, $flag) = (shift,shift) ;
  my $file = extract_single_parameter($flag, 1) ;
  return unless $file ;
  open FILE, ">", "$file" or die "can't write to $file: $!" ;
  my $q = '"' ;

  my $txt = join ("\n+$q,$q+ ",
    map {
      "$q" . $_->{tz} . "$q" .
      "  # " .  $_->{comment}
    } sort {$a->{tz} cmp $b->{tz}} @$list
  ) ;
  print FILE "# NEVER TOUCH THIS GENERATED FILE\n" ;
  print FILE "list =\n      $txt\n.\n" ;
  close FILE or die "can't write to $file: $!" ;
  print "Full list of geographical Olson names (", scalar @$list, " zones) is written to '$file'\n" ;
}


sub extract_single_parameter
{
  my ($flag, $optional) = (shift, shift) ;
  my $re = qr/^--$flag=(.+)$/ ;
  my @args = grep { m/$re/ } @ARGV ;
  return if $optional and not @args ;
  die "no or more than one --$flag=... given on command line\n" unless @args==1 ;
  die unless $args[0] =~ m/$re/ ;
  my $value = $1 ;
  @ARGV = grep { ! m/$re/ } @ARGV ;
  return $value ;
}

# Usage
sub usage
{
  print STDERR
    "Usage: $0 \\\n" .
    "       --olson=OLSON --mcc-main=MCC --distinct=DISTINCT --single=SINGLE \\\n" .
    "       --single-output=path --distinct-output=path --full-output=path \\\n" .
    "       WIKI_FILES\n" .
    "where\n" .
    "       OLSON: alternative zone.tab file (by default $DEFAULT_ZONE_TAB)\n" .
    "       MCC: file containing mcc list\n" .
    "       DISTINCT: file containing major zones of countries with simple zone structure\n" .
    "       SINGLE: file containing contries, which became single zone\n" .
    "       WIKI_FILES: list of files from wikipedia, only the last one is used\n" .
    "output:\n" .
    "       --single...: list of all single zone countries\n" .
    "       --distinct...: information about simple structure counries\n" .
    "       --full...: all Olson names\n" .
    "Any output is optional.\n" .
    "Example:\n" .
    "       ./prepare-timed-data.perl --mcc-main=MCC --distinct=distinct.tab --single=single.tab mcc-wiki*.html\n" .
    "Wiki files have to be downloaded from:\n" .
    "       http://en.wikipedia.org/w/index.php?title=List_of_mobile_country_codes&action=edit&section=1"
    .
    "\n" ;
}

__END__


sub output_multizones
{
  use DateTime ;
  use DateTime::TimeZone ;
  my ($tab, $flag) = (shift,shift) ;
  my $file = extract_single_parameter($flag, 1) ;
  # return unless $file ;

  for my $xy (sort keys %$tab)
  {
    my @list = @{$tab->{$xy}} ;
    { use Data::Dumper ;
    print scalar @list, Dumper(@list)  ; }
    next unless scalar @list > 1 ;
    for my $z (@list)
    {
      my $tz = $z->{tz} ;
      my $zz = DateTime::TimeZone->new(name=>$tz) ;
      print "$xy $tz ", $zz->offset_for_datetime(DateTime::now()), "\n" ;
    }
  }
}
