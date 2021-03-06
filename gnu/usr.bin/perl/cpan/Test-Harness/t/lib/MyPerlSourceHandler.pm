# subclass for testing customizing & subclassing

package MyPerlSourceHandler;

use strict;
use vars '@ISA';

use MyCustom;
use TAP::Parser::IteratorFactory;
use TAP::Parser::SourceHandler::Perl;

@ISA = qw( TAP::Parser::SourceHandler::Perl MyCustom );

TAP::Parser::IteratorFactory->register_handler(__PACKAGE__);

sub can_handle {
    my $class = shift;
    my $vote  = $class->SUPER::can_handle(@_);
    $vote += 0.1 if $vote > 0;    # steal the Perl handler's vote
    return $vote;
}

1;

