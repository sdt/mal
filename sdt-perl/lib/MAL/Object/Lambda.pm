package MAL::Object::Lambda;
use 5.20.0;
use warnings;

use parent qw( MAL::Object::Applyable );

use Function::Parameters qw( :strict );
use MAL::Environment;

fun index_of($list, $item) {
    for my $i (0 .. scalar(@$list)-1) {
        return $i if $list->[$i] eq $item;
    }
    return;
}

method is_macro {
    my $meta = $self->meta;
    return $meta->is_hash && $meta->contains(
        MAL::Object->string("ismacro"));
}

method new($class: $parameters, $expression, $env) {
    my $self = {
        expression => $expression,
        env => $env,
    };

    my @params = map { $_->value } $parameters->items;
    my $ampersand = index_of(\@params, '&');
    if (defined $ampersand) {
        die "Must be one and only one parameter after &"
            unless scalar(@params) == $ampersand + 2;
        $self->{slurpy} = pop(@params);
        pop(@params); # drop the '&'
    }
    $self->{parameters} = \@params;
    bless $self, $class;
}

method clone {
    my $clone = { %$self }; # single-level copy
    bless $clone, ref $self;
}

method apply($args) {
    my $inner = MAL::Environment->new(outer => $self->{env});
    for my $key (@{ $self->{parameters} }) {
        $inner->set($key, $args->car);
        $args = $args->cdr;
    }
    if (exists $self->{slurpy}) {
        $inner->set($self->{slurpy}, $args);
    }
    return ($self->{expression}, $inner);
}

method to_string {
    return '#function';
}

1;
