object M {
  function method()->() {
    //does nothing
  }
}

object M1 {

  function main()->() {
    x = M
    x.method()
  }
}
