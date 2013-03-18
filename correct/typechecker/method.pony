object M {
  function method(q:Int)->() {
    //does nothing
  }
}

object M1 {

  function main()->() {
    x = M
    x.method()
  }
}
