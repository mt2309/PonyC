object X {
  function method()->() {
    //does nothing
  }
}

object Y {

  function main()->() {
    x = X
    x.method()
  }
}
