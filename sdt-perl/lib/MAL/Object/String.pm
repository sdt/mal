package MAL::Object::String;
use 5.20.0;
use warnings;

use parent qw( MAL::Object::Scalar );
use Function::Parameters qw( :strict );

method to_string($readable = 0) {
    my $value = $self->value;
    if ($readable) {
        $value =~ s/\\/\\\\/g;
        $value =~ s/"/\\"/g;
        $value =~ s/\n/\\n/g;
        return '"' . $value . '"';
    }
    return $value;
}

method equal($rhs) {
    return $self->same_type($rhs) && ($self->value eq $rhs->value);
}

1;
