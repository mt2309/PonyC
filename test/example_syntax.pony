object Array[T]
{
  var _size:U32
  // MAGIC GOES HERE FOR THE STUFF

  new( _size = 0 )
  function( i:U32 )->( t:T ) throws
  function update( i:U32, t:T ) throws
  {
    // MAGIC: array[t] = t
  }
}

object Vector[T, d:U32]
object Matrix[T, x:U32, y:U32]

var foo = Array[U32]
foo( 6 )
foo( 6 ) = x
foo.update( 6, x )

wombat( "foo" ) = "bar"

wombat.other_update( "foo", "bar" )

match a
{
  case 0 | 1 { stuff }
  case Disjunction[U32]( 0, 1 ) { stuff }
}

match 1 + 2
{
  case 1 { stuff }
  case 1 as i:U32 { stuff }
  case as i:U32 { stuff }
  case { stuff }
}

match a, b
{
  case 1 as i:U32, 2 as j:U32 if bob.is_uncle() { stuff } // legal
  case 1 { stuff } // illegal
  case { stuff } // legal
}

object None
{
  new()
}

var n = None.string()
var n = None().string()

object Math
{
  function pi()->( F64( 3.141592 ) )
}

var pi = Math.pi()


thing => Foo, Bar

use Thing = "some/damn/thing"
use Thing2 = "http://server.org/some/damn/thing"

var foo = Thing::Foo[ar]
var foo2 = Thing2::Foo

lambda( args )->( results )

object Foo
{
  function stuff( a, b = 4, c = 6 )->( d ) { ... }
}


var f = Foo
var g = \(f.stuff( a )()).wibble() // : lambda( b = 4, c = 6 )->( d )

var h = \f.stuff( a ).womble()
var h:lambda( a, c = 6 )->( d ) = f.stuff( a )\womble()

var h = \womble()

var h = curry f.stuff( a ).womble()

f.stuff( a )()

FUTURE: womble( stuff( f, a ) )

g()
g( 1, 2 )
g( c = 7 )

var f = lambda!( args )->( results ) of { ... }


!f.thing().other() -> !(f.thing().other())

var a, _ = stuff()

