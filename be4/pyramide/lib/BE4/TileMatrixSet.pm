# Copyright © (2011) Institut national de l'information
#                    géographique et forestière 
# 
# Géoportail SAV <geop_services@geoportail.fr>
# 
# This software is a computer program whose purpose is to publish geographic
# data using OGC WMS and WMTS protocol.
# 
# This software is governed by the CeCILL-C license under French law and
# abiding by the rules of distribution of free software.  You can  use, 
# modify and/ or redistribute the software under the terms of the CeCILL-C
# license as circulated by CEA, CNRS and INRIA at the following URL
# "http://www.cecill.info". 
# 
# As a counterpart to the access to the source code and  rights to copy,
# modify and redistribute granted by the license, users are provided only
# with a limited warranty  and the software's author,  the holder of the
# economic rights,  and the successive licensors  have only  limited
# liability. 
# 
# In this respect, the user's attention is drawn to the risks associated
# with loading,  using,  modifying and/or developing or reproducing the
# software by the user in light of its specific status of free software,
# that may mean  that it is complicated to manipulate,  and  that  also
# therefore means  that it is reserved for developers  and  experienced
# professionals having in-depth computer knowledge. Users are therefore
# encouraged to load and test the software's suitability as regards their
# requirements in conditions enabling the security of their systems and/or 
# data to be ensured and,  more generally, to use and operate it in the 
# same conditions as regards security. 
# 
# The fact that you are presently reading this means that you have had
# 
# knowledge of the CeCILL-C license and that you accept its terms.

package BE4::TileMatrixSet;

use strict;
use warnings;

use Log::Log4perl qw(:easy);

use XML::Simple;
use Data::Dumper;
use Geo::OSR;

use BE4::TileMatrix;

require Exporter;
use AutoLoader qw(AUTOLOAD);

our @ISA = qw(Exporter);

our %EXPORT_TAGS = ( 'all' => [ qw() ] );
our @EXPORT_OK   = ( @{$EXPORT_TAGS{'all'}} );
our @EXPORT      = qw();

################################################################################
# Constantes
use constant TRUE  => 1;
use constant FALSE => 0;

################################################################################
=begin nd
Group: variable

variable: $self
    * PATHFILENAME
    * name
    * filename
    * filepath
    
    * levelsBind - hash binding Tile matrix identifiants (string) and order (integer) in ascending resolutions
    * topID
    * topResolution
    * bottomID
    * bottomResolution
    
    * srs - casted in uppercase
    * coordinatesInversion : boolean - to know if we have to reverse coordinates to harvest in this SRS
    * tileMatrix : hash of BE4::TileMatrix
    * isQTree : boolean
=cut

####################################################################################################
#                                       CONSTRUCTOR METHODS                                        #
####################################################################################################

# Group: constructor

sub new {
  my $this = shift;

  my $class= ref($this) || $this;
  # IMPORTANT : if modification, think to update natural documentation (just above) and pod documentation (bottom)
  my $self = {
    PATHFILENAME => undef,
    name     => undef,
    filename => undef,
    filepath => undef,
    #
    levelsBind => undef,
    topID => undef,
    topResolution => undef,
    bottomID => undef,
    bottomResolution  => undef,
    #
    srs        => undef,
    coordinatesInversion  => FALSE,
    tileMatrix => {},
    #
    isQTree => undef,
  };

  bless($self, $class);
  
  TRACE;
  
  # init. class
  return undef if (! $self->_init(@_));
  # load 
  return undef if (! $self->_load());
  
  return $self;
}

sub _init {
    my $self     = shift;
    my $pathfile = shift;

    TRACE;
    
    return FALSE if (! defined $pathfile);
    
    if (! -f $pathfile) {
      ERROR ("File TMS doesn't exist !");
      return FALSE;
    }
    
    # init. params    
    $self->{PATHFILENAME}=$pathfile;
    
    #
    $self->{filepath} = File::Basename::dirname($pathfile);
    $self->{filename} = File::Basename::basename($pathfile);
    $self->{name}     = File::Basename::basename($pathfile);
    $self->{name}     =~ s/\.(tms|TMS)$//;
    
    TRACE (sprintf "name : %s", $self->{name});
    
    return TRUE;
}

sub _load {
    my $self = shift;
    
    TRACE;
    
    my $xmltms  = new XML::Simple(KeepRoot => 0, SuppressEmpty => 1, ContentKey => '-content');
    my $xmltree = eval { $xmltms->XMLin($self->{PATHFILENAME}); };
    
    if ($@) {
        ERROR (sprintf "Can not read the XML file TMS : %s !", $@);
        return FALSE;
    }
  
    # load tileMatrix
    while (my ($k,$v) = each %{$xmltree->{tileMatrix}}) {
        # we identify level max (with the best resolution, the smallest) and level min (with the 
        # worst resolution, the biggest)
        
        if (! defined $self->{topID} || ! defined $self->{topResolution} || $v->{resolution} > $self->{topResolution}) {
            $self->{topID} = $k;
            $self->{topResolution} = $v->{resolution};
        }
        if (! defined $self->{bottomID} || ! defined $self->{bottomResolution} || $v->{resolution} < $self->{bottomResolution}) {
            $self->{bottomID} = $k;
            $self->{bottomResolution} = $v->{resolution};
        }
        
        my $objTM = BE4::TileMatrix->new({
            id => $k,
            resolution     => $v->{resolution},
            topLeftCornerX => $v->{topLeftCornerX},
            topLeftCornerY => $v->{topLeftCornerY},
            tileWidth      => $v->{tileWidth},
            tileHeight     => $v->{tileHeight},
            matrixWidth    => $v->{matrixWidth},
            matrixHeight   => $v->{matrixHeight},
        });
        
        if (! defined $objTM) {
            ERROR(sprintf "Cannot create the TileMatrix object for the level '%s'",$k);
            return FALSE;
        }
        
        $self->{tileMatrix}->{$k} = $objTM;
        undef $objTM;
    }
    
    if (! $self->getCountTileMatrix()) {
        ERROR (sprintf "No tile matrix loading from XML file TMS !");
        return FALSE;
    }
    
    # srs (== crs)
    if (! exists ($xmltree->{crs}) || ! defined ($xmltree->{crs})) {
        ERROR (sprintf "Can not determine parameter 'srs' in the XML file TMS !");
        return FALSE;
    }
    $self->{srs} = uc($xmltree->{crs}); # srs is cast in uppercase in order to ease comparisons
    
    # Have coodinates to be reversed ?
    my $sr= new Geo::OSR::SpatialReference;
    eval { $sr->ImportFromProj4('+init='.$self->{srs}.' +wktext'); };
    if ($@) {
        eval { $sr->ImportFromProj4('+init='.lc($self->{srs}).' +wktext'); };
        if ($@) {
            ERROR("$@");
            ERROR (sprintf "Impossible to initialize the final spatial coordinate system (%s) to know if coordinates have to be reversed !\n",$self->{srs});
            return FALSE;
        }
    }

    my $authority = (split(":",$self->{srs}))[0];
    if ($sr->IsGeographic() && uc($authority) eq "EPSG") {
        INFO(sprintf "Coordinates will be reversed in requests (SRS : %s)",$self->{srs});
        $self->{coordinatesInversion} = TRUE;
    } else {
        INFO(sprintf "Coordinates order will be kept in requests (SRS : %s)",$self->{srs});
        $self->{coordinatesInversion} = FALSE;
    }
    
    # clean
    $xmltree = undef;
    $xmltms  = undef;
    
    # tileMatrix list sort by resolution
    my @tmList = $self->getTileMatrixByArray();
  
    # Is TMS a QuadTree ? If not, we use a graph (less efficient for calculs)
    $self->{isQTree} = TRUE; # default value
    if (scalar(@tmList) != 1) {
        my $epsilon = $tmList[0]->getResolution / 100 ;
        for (my $i = 0; $i < scalar(@tmList) - 1;$i++) {
            if ( abs($tmList[$i]->getResolution*2 - $tmList[$i+1]->getResolution) > $epsilon ) {
                $self->{isQTree} = FALSE;
                INFO(sprintf "Not a QTree : resolutions don't go by twos : level '%s' (%s) and level '%s' (%s) are not valid",
                    $tmList[$i]->{id},$tmList[$i]->getResolution,
                    $tmList[$i+1]->{id},$tmList[$i+1]->getResolution);
                last;
            }
            elsif ( abs($tmList[$i]->getTopLeftCornerX - $tmList[$i+1]->getTopLeftCornerX) > $epsilon ) {
                $self->{isQTree} = FALSE;
                ERROR(sprintf "Not a QTree : 'topleftcornerx' is not the same for all levels : level '%s' (%s) and level '%s' (%s) are not valid",
                    $tmList[$i]->{id},$tmList[$i]->getTopLeftCornerX,
                    $tmList[$i+1]->{id},$tmList[$i+1]->getTopLeftCornerX);
                last;
            }
            elsif ( abs($tmList[$i]->getTopLeftCornerY - $tmList[$i+1]->getTopLeftCornerY) > $epsilon ) {
                $self->{isQTree} = FALSE;
                ERROR(sprintf "Not a QTree : 'topleftcornery' is not the same for all levels : level '%s' (%s) and level '%s' (%s) are not valid",
                    $tmList[$i]->{id},$tmList[$i]->getTopLeftCornerY,
                    $tmList[$i+1]->{id},$tmList[$i+1]->getTopLeftCornerY);
                last;
            }
            elsif ( $tmList[$i]->getTileWidth != $tmList[$i+1]->getTileWidth) {
                $self->{isQTree} = FALSE;
                ERROR(sprintf "Not a QTree : 'tilewidth' is not the same for all levels : level '%s' (%s) and level '%s' (%s) are not valid",
                    $tmList[$i]->{id},$tmList[$i]->getTileWidth,
                    $tmList[$i+1]->{id},$tmList[$i+1]->getTileWidth);
                last;
            }
            elsif ( $tmList[$i]->getTileHeight != $tmList[$i+1]->getTileHeight) {
                $self->{isQTree} = FALSE;
                INFO(sprintf "Not a QTree : 'tileheight' is not the same for all levels : level '%s' (%s) and level '%s' (%s) are not valid",
                    $tmList[$i]->{id},$tmList[$i]->getTileHeight,
                    $tmList[$i+1]->{id},$tmList[$i+1]->getTileHeight);
                last;
            }
        };
    };
  
    # on fait un hash pour retrouver l'ordre d'un niveau a partir de son id.
    for (my $i=0; $i < scalar @tmList; $i++){
        $self->{levelsBind}{$tmList[$i]->getID()} = $i;
    }
    
    if ($self->isQTree) { return TRUE;}
    ## Adding informations about child/parent in TM objects
    for (my $i = 0; $i < scalar(@tmList) ;$i++) {
        if (! $self->computeTmSource($tmList[$i])) {
            ERROR(sprintf "Nor a QTree neither a Graph made for nearest neighbour generation. No source for level %s.",$tmList[$i]->getID());
            return FALSE;
        }
    }
    
    return TRUE;
}

####################################################################################################
#                                       GETTERS / SETTERS                                          #
####################################################################################################

# Group: getters - setters

sub getPathFilename {
    my $self = shift;
    return $self->{PATHFILENAME};
}

sub getSRS {
  my $self = shift;
  return $self->{srs};
}
sub getInversion {
  my $self = shift;
  return $self->{coordinatesInversion};
}
sub getName {
  my $self = shift;
  return $self->{name};
}
sub getPath {
  my $self = shift;
  return $self->{filepath};
}
sub getFile {
  my $self = shift;
  return $self->{filename};
}

sub getLevelTop {
  my $self = shift;
  return $self->{topID};
}

sub getLevelsBind {
  my $self = shift;
  return $self->{levelsBind};
}

sub getLevelBottom {
  my $self = shift;
  return $self->{bottomID};
}

sub getLevelTopResolution {
  my $self = shift;
  return $self->{topResolution};
}

sub getLevelBottomResolution {
  my $self = shift;
  return $self->{bottomResolution};
}

# TileWidth TileHeight
sub getTileWidth {
  my $self = shift;
  my $levelID = shift;
  
  $levelID = $self->{bottomID} if (! defined $levelID);
  
  # size of tile in pixel !
  return $self->{tileMatrix}->{$levelID}->getTileWidth;
}
sub getTileHeight {
  my $self = shift;
  my $levelID = shift;
  
  $levelID = $self->{bottomID} if (! defined $levelID);
  
  # size of tile in pixel !
  return $self->{tileMatrix}->{$levelID}->getTileHeight;
}

sub isQTree {
    my $self = shift;
    return $self->{isQTree};
}

####################################################################################################
#                                    TILE MATRIX MANAGER                                           #
####################################################################################################

# Group: tile matrix manager

#
=begin nd
method: computeTmSource

Parameters:
    tmTarget - a BE4::TileMatrix object.

Returns:
    FALSE if there is no TM source for TM target (unless TM target is BotttomTM) 
    TRUE if there is a TM source (obj) for the TM target (obj) in argument.
=cut
sub computeTmSource {
  my $self = shift;
  my $tmTarget = shift;
  
  if ($tmTarget->{id} == $self->{bottomID}) {
    return TRUE;
  }

  # The TM to be used to compute images in TM Parent
  my $tmSource = undef;
  
  # position du pixel en haut à gauche
  my $xTopLeftCorner_CenterPixel = $tmTarget->getTopLeftCornerX() + 0.5 * $tmTarget->getResolution();
  my $yTopLeftCorner_CenterPixel = $tmTarget->getTopLeftCornerY() - 0.5 * $tmTarget->getResolution();

  for (my $i = $self->getOrderfromID($tmTarget->getID()) - 1; $i >= $self->getOrderfromID($self->getLevelBottom) ;$i--) {
      my $potentialTmSource = $self->getTileMatrix($self->getIDfromOrder($i));
      # la précision vaut 1/100 de la plus petit résolution du TMS
      my $epsilon = $self->getTileMatrix($self->getLevelBottom())->getResolution() / 100;
      my $rapport = $tmTarget->getResolution() / $potentialTmSource->getResolution() ;
      #on veut que le rapport soit (proche d') un entier
      next if ( abs( int( $rapport + 0.5) - $rapport) > $epsilon );
      # on veut que les pixels soient superposables (pour faire une interpolation nn)
      # on regarde le pixel en haut à gauche de tmtarget
      # on verfie qu'il y a bien un pixel correspondant dans tmpotentialsource
      my $potentialTm_xTopLeftCorner_CenterPixel = $potentialTmSource->getTopLeftCornerX() + 0.5 * $potentialTmSource->getResolution() ;
      next if (abs($xTopLeftCorner_CenterPixel - $potentialTm_xTopLeftCorner_CenterPixel) > $epsilon );
      my $potentialTm_yTopLeftCorner_CenterPixel = $potentialTmSource->getTopLeftCornerY() - 0.5 * $potentialTmSource->getResolution() ;
      next if (abs($yTopLeftCorner_CenterPixel - $potentialTm_yTopLeftCorner_CenterPixel) > $epsilon );
      $tmSource = $potentialTmSource;
      last;
  }
  
  # si on n'a rien trouvé, on sort en erreur
  if (!  defined $tmSource) {
     return FALSE;
  }
  
  $tmSource->addTargetsTm($tmTarget);
  
  return TRUE;
}


#
=begin nd
method: getTileMatrixByArray

Returns:
    The tile matrix array in the ascending resolution order.
=cut
sub getTileMatrixByArray {
    my $self = shift;

    TRACE("sort by Resolution...");
    
    my @levels;

    foreach my $k (sort {$a->getResolution() <=> $b->getResolution()} (values %{$self->{tileMatrix}})) {
        push @levels, $k;
    }

    return @levels;
}

#
=begin nd
method: getTileMatrix

Returns:
    The tile matrix from the supplied ID. This ID is the TMS ID (string) and not the ascending resolution order (integer).
=cut
sub getTileMatrix {
  my $self = shift;
  my $level= shift; # id !
  
  if (! defined $level) {
    return undef;
  }
  
  return undef if (! exists($self->{tileMatrix}->{$level}));
  
  return $self->{tileMatrix}->{$level};
}

#
=begin nd
method: getCountTileMatrix

Returns:
    The count of tile matrix in the TMS.
=cut
sub getCountTileMatrix {
  my $self = shift;

  return scalar (keys %{$self->{tileMatrix}});
}

#
=begin nd
method: getIDfromOrder

Return the tile matrix ID from the ascending resolution order (integer).  
    - 0 (bottom level, smallest resolution)
    - NumberOfTM (top level, biggest resolution).

=cut
sub getIDfromOrder {
    my $self = shift;
    my $order= shift; 

    TRACE;

    foreach my $k (keys %{$self->{levelsBind}}) {
        if ($self->{levelsBind}->{$k} == $order) {return $k;}
    }

    return undef;
}

#
=begin nd
method: getBelowLevelID

Return:
    The tile matrix ID below the given tile matrix (ID).
=cut
sub getBelowLevelID {
    my $self = shift;
    my $ID= shift; 

    TRACE;
    
    return undef if (! exists $self->{levelsBind}->{$ID});
    my $order = $self->{levelsBind}->{$ID};
    return undef if ($order == 0);
    return $self->getIDfromOrder($order-1);
}

#
=begin nd
method: getOrderfromID

Return the tile matrix order from the ID.
    - 0 (bottom level, smallest resolution)
    - NumberOfTM (top level, biggest resolution).
=cut
sub getOrderfromID {
    my $self = shift;
    my $ID= shift; 

    TRACE;

    if (exists $self->{levelsBind}->{$ID}) {
        return $self->{levelsBind}->{$ID};
    } else {
        return undef;
    }
}

####################################################################################################
#                                       DEBUG METHODS                                              #
####################################################################################################

# Group: DEBUG METHODS

#
=begin nd
method: exportForDebug

Print attributs of the TileMatrixSet

=cut
sub exportForDebug {
    my $self = shift;
    
    my $output = "";
    
    $output .= sprintf "\nObject BE4::TileMatrixSet :\n";
    if (defined $self->getPathFilename() ) {
        $output .= sprintf "\t %s : %s\n","PATHFILENAME",$self->getPathFilename();
    };
    if (defined $self->getName() ) {
        $output .= sprintf "\t %s : %s\n","name",$self->getName();
    };
    if (defined $self->getPath() ) {
        $output .= sprintf "\t %s : %s\n","filepath",$self->getPath();
    };
    if (defined $self->getFile() ) {
        $output .= sprintf "\t %s : %s\n","filename",$self->getFile();
    };
    if (defined $self->getLevelTop() ) {
        $output .= sprintf "\t %s : %s\n","topID",$self->getLevelTop();
    };
    if (defined $self->getLevelTopResolution() ) {
        $output .= sprintf "\t %s : %s\n","topResolution",$self->getLevelTopResolution();
    };
    if (defined $self->getLevelBottom() ) {
        $output .= sprintf "\t %s : %s\n","bottomID",$self->getLevelBottom();
    };
    if (defined $self->getLevelBottomResolution() ) {
        $output .= sprintf "\t %s : %s\n","bottomResolution",$self->getLevelBottomResolution();
    };
    if (defined $self->getSRS() ) {
        $output .= sprintf "\t %s : %s\n","SRS",$self->getSRS();
    };
    if (defined $self->getInversion() ) {
        if ( $self->getInversion() ) {
            $output .= sprintf "\t %s value is %s\n","coordinatesInversion","TRUE";
        } else {
            $output .= sprintf "\t %s value is %s\n","coordinatesInversion","FALSE";
        }
    };
    if (defined $self->isQTree() ) {
        if ( $self->isQTree() ) {
            $output .= sprintf "\t %s value is %s\n","isQTree","TRUE";
        } else {
            $output .= sprintf "\t %s value is %s\n","isQTree","FALSE";
        }
    };
    $output .= sprintf "\t levelsBind hash :\n";
    foreach my $key (keys($self->getLevelsBind())) {
        $output .= sprintf "\t\t ID : %s ; Order : %s .\n",$key,$self->getLevelsBind()->{$key};
    }
    $output .= sprintf "\t TileMatrix Array :\n";
    foreach my $tm (@{$self->getTileMatrixByArray()}) {
        $output .= sprintf "\t\t TM ID : %s, TM Resolution : %s .\n",$tm->getID(),$tm->getResolution();
    }    
    
    return $output;
}

1;
__END__

=head1 NAME

BE4::TileMatrixSet - load a file tile matrix set.

=head1 SYNOPSIS

    use BE4::TileMatrixSet;
    
    my $filepath = "/home/ign/tms/LAMB93_50cm.tms";
    my $objTMS = BE4::TileMatrixSet->new($filepath);
    
    $objTMS->getTileMatrixCount()};      # ie 19
    $objTMS->getTileMatrix(12);          # object TileMatrix with level id = 12
    $objTMS->getSRS();                   # ie 'IGNF:LAMB93'
    $objTMS->getName();                  # ie 'LAMB93_50cm'
    $objTMS->getFile();                  # ie 'LAMB93_50cm.tms'
    $objTMS->getPath();                  # ie '/home/ign/tms/'
    
=head1 DESCRIPTION

=head2 ATTRIBUTES

=over 4

=item PATHFILENAME

Complete file path : /path/to/SRS_RES.tms

=item name, filename, filepath

Split PATHFILENAME : name : SRS_RES, filename : SRS_RES.tms, filepath : /path/to.

=item levelsBind

Hash binding Tile matrix identifiant (string) and order (integer) in ascending resolutions.

=item topID, topResolution

Higher level ID and its resolution.

=item bottomID, bottomResolution

Lower level ID and its resolution.

=item srs

=item isQTree

TRUE if this TMS describe a quad tree, FALSE otherwise.

=item coordinatesInversion

For some SRS, we have to reverse coordinates when we compose WMS request (1.3.0). Used test to determine this SRSs is : if the SRS is geographic and an EPSG one.

=item tileMatrix

Hash of TileMatrix objects : levelID => objTileMatrix

=back

=head1 SAMPLE

Sample TMS file (F<LAMB93_50cm.tms>)

    <tileMatrixSet>
	<crs>IGNF:LAMB93</crs>
	<tileMatrix>
            <id>0</id>
            <resolution>131072</resolution>
            <topLeftCornerX> 0 </topLeftCornerX>
            <topLeftCornerY> 12000000 </topLeftCornerY>
            <tileWidth>256</tileWidth>
            <tileHeight>256</tileHeight>
            <matrixWidth>1</matrixWidth>
            <matrixHeight>1</matrixHeight>
	</tileMatrix>
	<tileMatrix>
            <id>1</id>
            <resolution>65536</resolution>
            <topLeftCornerX> 0 </topLeftCornerX>
            <topLeftCornerY> 12000000 </topLeftCornerY>
            <tileWidth>256</tileWidth>
            <tileHeight>256</tileHeight>
            <matrixWidth>1</matrixWidth>
            <matrixHeight>1</matrixHeight>
	</tileMatrix>
        .
        .
        .
        <tileMatrix>
            <id>17</id>
            <resolution>1</resolution>
            <topLeftCornerX> 0 </topLeftCornerX>
            <topLeftCornerY> 12000000 </topLeftCornerY>
            <tileWidth>256</tileWidth>
            <tileHeight>256</tileHeight>
            <matrixWidth>5040</matrixWidth>
            <matrixHeight>42040</matrixHeight>
        </tileMatrix>
	<tileMatrix>
            <id>18</id>
            <resolution>0.5</resolution>
            <topLeftCornerX> 0 </topLeftCornerX>
            <topLeftCornerY> 12000000 </topLeftCornerY>
            <tileWidth>256</tileWidth>
            <tileHeight>256</tileHeight>
            <matrixWidth>10080</matrixWidth>
            <matrixHeight>84081</matrixHeight>
        </tileMatrix>
    </tileMatrixSet>

=head1 LIMITATIONS AND BUGS

File name of tms must be with extension : tms or TMS.

All levels must be continuous (QuadTree) and unique.

=head1 SEE ALSO

=head2 POD documentation

=begin html

<ul>
<li><A HREF="./lib-BE4-TileMatrix.html">BE4::TileMatrix</A></li>
</ul>

=end html

=head2 NaturalDocs

=begin html

<A HREF="../Natural/Html/index.html">Index</A>

=end html

=head1 AUTHOR

Bazonnais Jean Philippe, E<lt>jean-philippe.bazonnais@ign.frE<gt>

=head1 COPYRIGHT AND LICENSE

Copyright (C) 2011 by Bazonnais Jean Philippe

This library is free software; you can redistribute it and/or modify it under the same terms as Perl itself, either Perl version 5.10.1 or, at your option, any later version of Perl 5 you may have available.

=cut
