
use lib 't';
use strict;
use warnings;
use bytes;

use Test::More ;
use CompTestUtils;

our ($extra);

BEGIN {
    # use Test::NoWarnings, if available
    $extra = 0 ;
    $extra = 1
        if eval { require Test::NoWarnings ;  import Test::NoWarnings; 1 };

}

sub run
{

    my $CompressClass   = identify();
    my $UncompressClass = getInverse($CompressClass);
    my $Error           = getErrorRef($CompressClass);
    my $UnError         = getErrorRef($UncompressClass);



    my $hello = <<EOM ;
hello world
this is a test
some more stuff on this line
ad finally...
EOM

    print "#\n# Testing $UncompressClass\n#\n";

    my $compressed = mkComplete($CompressClass, $hello);
    my $cc = $compressed ;

    plan tests => (length($compressed) * 6 * 7) + 1 + $extra ;

    is anyUncompress(\$cc), $hello ;

    for my $blocksize (1, 2, 13)
    {
        for my $i (0 .. length($compressed) - 1)
        {
            for my $useBuf (0 .. 1)
            {
                print "#\n# BlockSize $blocksize, Length $i, Buffer $useBuf\n#\n" ;
                my $lex = new LexFile my $name ;
        
                my $prime = substr($compressed, 0, $i);
                my $rest = substr($compressed, $i);
        
                my $start  ;
                if ($useBuf) {
                    $start = \$rest ;
                }
                else {
                    $start = $name ;
                    writeFile($name, $rest);
                }

                #my $gz = new $UncompressClass $name,
                my $gz = new $UncompressClass $start,
                                              -Append      => 1,
                                              -BlockSize   => $blocksize,
                                              -Prime       => $prime,
                                              -Transparent => 0
                                              ;
                ok $gz;
                ok ! $gz->error() ;
                my $un ;
                my $status = 1 ;
                $status = $gz->read($un) while $status > 0 ;
                is $status, 0 ;
                ok ! $gz->error() 
                    or print "Error is '" . $gz->error() . "'\n";
                is $un, $hello ;
                ok $gz->eof() ;
                ok $gz->close() ;
            }
        }
    }
}
 
1;
