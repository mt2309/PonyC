use Array = Primitive::Array

trait Iterable[T] {
  function hasNext()->(x:Boolean)
  function next()->(x:T) throws
  function remove()
}

trait List[T] is Iterable[T] {


}

object Node[T] is List {

}

object Nil is List {

}
