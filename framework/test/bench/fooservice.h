
// A dummy interface and impl to use with service trackers
namespace googlebenchmark {
namespace test {
class Foo
{
public:
  virtual ~Foo() = default;
};

class FooImpl : public Foo
{};
}
}
