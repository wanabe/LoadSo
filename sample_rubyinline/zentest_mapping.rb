module ZenTestMapping
  def test_to_normal(name)
    name = name.dup
    name.sub!(/^test_/, "")
    name.sub!(/_bang$/, "!")
    name
  end
end
