// Adapted from the following:
//===- llvm/unittest/ADT/CompactVectorTest.cpp ------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// TCompactVector unit tests.
//
//===----------------------------------------------------------------------===//

#include <library/cpp/yt/compact_containers/compact_vector.h>

#include <library/cpp/testing/gtest/gtest.h>

#include <util/string/cast.h>

#include <algorithm>
#include <list>

#include <stdarg.h>

namespace NYT {
namespace {

////////////////////////////////////////////////////////////////////////////////

/// A helper class that counts the total number of constructor and
/// destructor calls.
class Constructable {
private:
  static int numConstructorCalls;
  static int numMoveConstructorCalls;
  static int numCopyConstructorCalls;
  static int numDestructorCalls;
  static int numAssignmentCalls;
  static int numMoveAssignmentCalls;
  static int numCopyAssignmentCalls;

  bool constructed;
  int value;

public:
  Constructable() : constructed(true), value(0) {
    ++numConstructorCalls;
  }

  Constructable(int val) : constructed(true), value(val) {
    ++numConstructorCalls;
  }

  Constructable(const Constructable & src) : constructed(true) {
    value = src.value;
    ++numConstructorCalls;
    ++numCopyConstructorCalls;
  }

  Constructable(Constructable && src) : constructed(true) {
    value = src.value;
    ++numConstructorCalls;
    ++numMoveConstructorCalls;
  }

  ~Constructable() {
    EXPECT_TRUE(constructed);
    ++numDestructorCalls;
    constructed = false;
  }

  Constructable & operator=(const Constructable & src) {
    EXPECT_TRUE(constructed);
    value = src.value;
    ++numAssignmentCalls;
    ++numCopyAssignmentCalls;
    return *this;
  }

  Constructable & operator=(Constructable && src) {
    EXPECT_TRUE(constructed);
    value = src.value;
    ++numAssignmentCalls;
    ++numMoveAssignmentCalls;
    return *this;
  }

  int getValue() const {
    return abs(value);
  }

  static void reset() {
    numConstructorCalls = 0;
    numMoveConstructorCalls = 0;
    numCopyConstructorCalls = 0;
    numDestructorCalls = 0;
    numAssignmentCalls = 0;
    numMoveAssignmentCalls = 0;
    numCopyAssignmentCalls = 0;
  }

  static int getNumConstructorCalls() {
    return numConstructorCalls;
  }

  static int getNumMoveConstructorCalls() {
    return numMoveConstructorCalls;
  }

  static int getNumCopyConstructorCalls() {
    return numCopyConstructorCalls;
  }

  static int getNumDestructorCalls() {
    return numDestructorCalls;
  }

  static int getNumAssignmentCalls() {
    return numAssignmentCalls;
  }

  static int getNumMoveAssignmentCalls() {
    return numMoveAssignmentCalls;
  }

  static int getNumCopyAssignmentCalls() {
    return numCopyAssignmentCalls;
  }

  friend bool operator==(const Constructable & c0, const Constructable & c1) {
    return c0.getValue() == c1.getValue();
  }
};

int Constructable::numConstructorCalls;
int Constructable::numCopyConstructorCalls;
int Constructable::numMoveConstructorCalls;
int Constructable::numDestructorCalls;
int Constructable::numAssignmentCalls;
int Constructable::numCopyAssignmentCalls;
int Constructable::numMoveAssignmentCalls;

struct NonCopyable {
  NonCopyable() {}
  NonCopyable(NonCopyable &&) {}
  NonCopyable &operator=(NonCopyable &&) { return *this; }
private:
  NonCopyable(const NonCopyable &) = delete;
  NonCopyable &operator=(const NonCopyable &) = delete;
};

[[maybe_unused]] void CompileTest() {
  TCompactVector<NonCopyable, 0> V;
  V.resize(42);
}

class CompactVectorTestBase : public testing::Test {
protected:

  void SetUp() override {
    Constructable::reset();
  }

  template <typename VectorT>
  void assertEmpty(VectorT & v) {
    // Size tests
    EXPECT_EQ(0u, v.size());
    EXPECT_TRUE(v.empty());

    // Iterator tests
    EXPECT_TRUE(v.begin() == v.end());
  }

  // Assert that v contains the specified values, in order.
  template <typename VectorT>
  void assertValuesInOrder(VectorT & v, size_t size, ...) {
    EXPECT_EQ(size, v.size());

    va_list ap;
    va_start(ap, size);
    for (size_t i = 0; i < size; ++i) {
      int value = va_arg(ap, int);
      EXPECT_EQ(value, v[i].getValue());
    }

    va_end(ap);
  }

  // Generate a sequence of values to initialize the vector.
  template <typename VectorT>
  void makeSequence(VectorT & v, int start, int end) {
    for (int i = start; i <= end; ++i) {
      v.push_back(Constructable(i));
    }
  }
};

// Test fixture class
template <typename VectorT>
class CompactVectorTest : public CompactVectorTestBase {
protected:
  VectorT theVector;
  VectorT otherVector;
};


using CompactVectorTestTypes = ::testing::Types<TCompactVector<Constructable, 0>,
                         TCompactVector<Constructable, 1>,
                         TCompactVector<Constructable, 2>,
                         TCompactVector<Constructable, 4>,
                         TCompactVector<Constructable, 5>
                         >;
TYPED_TEST_SUITE(CompactVectorTest, CompactVectorTestTypes);

// New vector test.
TYPED_TEST(CompactVectorTest, EmptyVectorTest) {
  SCOPED_TRACE("EmptyVectorTest");
  this->assertEmpty(this->theVector);
  EXPECT_TRUE(this->theVector.rbegin() == this->theVector.rend());
  EXPECT_EQ(0, Constructable::getNumConstructorCalls());
  EXPECT_EQ(0, Constructable::getNumDestructorCalls());
}

// Simple insertions and deletions.
TYPED_TEST(CompactVectorTest, PushPopTest) {
  SCOPED_TRACE("PushPopTest");

  // Track whether the vector will potentially have to grow.
  bool RequiresGrowth = this->theVector.capacity() < 3;

  // Push an element
  this->theVector.push_back(Constructable(1));

  // Size tests
  this->assertValuesInOrder(this->theVector, 1u, 1);
  EXPECT_FALSE(this->theVector.begin() == this->theVector.end());
  EXPECT_FALSE(this->theVector.empty());

  // Push another element
  this->theVector.push_back(Constructable(2));
  this->assertValuesInOrder(this->theVector, 2u, 1, 2);

  // Insert at beginning
  this->theVector.insert(this->theVector.begin(), this->theVector[1]);
  this->assertValuesInOrder(this->theVector, 3u, 2, 1, 2);

  // Pop one element
  this->theVector.pop_back();
  this->assertValuesInOrder(this->theVector, 2u, 2, 1);

  // Pop remaining elements
  this->theVector.pop_back();
  this->theVector.pop_back();
  this->assertEmpty(this->theVector);

  // Check number of constructor calls. Should be 2 for each list element,
  // one for the argument to push_back, one for the argument to insert,
  // and one for the list element itself.
  if (!RequiresGrowth) {
    EXPECT_EQ(5, Constructable::getNumConstructorCalls());
    EXPECT_EQ(5, Constructable::getNumDestructorCalls());
  } else {
    // If we had to grow the vector, these only have a lower bound, but should
    // always be equal.
    EXPECT_LE(5, Constructable::getNumConstructorCalls());
    EXPECT_EQ(Constructable::getNumConstructorCalls(),
              Constructable::getNumDestructorCalls());
  }
}

TYPED_TEST(CompactVectorTest, InsertEnd)
{
    SCOPED_TRACE("InsertEnd");

    TCompactVector<std::string, 5> vector;
    for (int index = 0; index < 10; ++index) {
        vector.insert(vector.end(), ToString(index));
    }
    for (int index = 0; index < 10; ++index) {
        EXPECT_EQ(vector[index], ToString(index));
    }
}

TYPED_TEST(CompactVectorTest, ShrinkToSmall)
{
    SCOPED_TRACE("ShrinkToSmall");

    TCompactVector<std::string, 5> vector;
    for (int index = 0; index < 10; ++index) {
        vector.shrink_to_small();
        vector.push_back(ToString(index));
    }

    for (int index = 0; index < 6; ++index) {
        vector.pop_back();
    }

    EXPECT_EQ(std::ssize(vector), 4);
    EXPECT_GE(static_cast<int>(vector.capacity()), 10);
    vector.shrink_to_small();
    EXPECT_EQ(std::ssize(vector), 4);
    EXPECT_EQ(static_cast<int>(vector.capacity()), 5);
    for (int index = 0; index < 4; ++index) {
        EXPECT_EQ(vector[index], ToString(index));
    }
}

// Clear test.
TYPED_TEST(CompactVectorTest, ClearTest) {
  SCOPED_TRACE("ClearTest");

  this->theVector.reserve(2);
  this->makeSequence(this->theVector, 1, 2);
  this->theVector.clear();

  this->assertEmpty(this->theVector);
  EXPECT_EQ(4, Constructable::getNumConstructorCalls());
  EXPECT_EQ(4, Constructable::getNumDestructorCalls());
}

// Resize smaller test.
TYPED_TEST(CompactVectorTest, ResizeShrinkTest) {
  SCOPED_TRACE("ResizeShrinkTest");

  this->theVector.reserve(3);
  this->makeSequence(this->theVector, 1, 3);
  this->theVector.resize(1);

  this->assertValuesInOrder(this->theVector, 1u, 1);
  EXPECT_EQ(6, Constructable::getNumConstructorCalls());
  EXPECT_EQ(5, Constructable::getNumDestructorCalls());
}

// Resize bigger test.
TYPED_TEST(CompactVectorTest, ResizeGrowTest) {
  SCOPED_TRACE("ResizeGrowTest");

  this->theVector.resize(2);

  EXPECT_EQ(2, Constructable::getNumConstructorCalls());
  EXPECT_EQ(0, Constructable::getNumDestructorCalls());
  EXPECT_EQ(2u, this->theVector.size());
}

TYPED_TEST(CompactVectorTest, ResizeWithElementsTest) {
  this->theVector.resize(2);

  Constructable::reset();

  this->theVector.resize(4);

  size_t Ctors = Constructable::getNumConstructorCalls();
  EXPECT_TRUE(Ctors == 2 || Ctors == 4);
  size_t MoveCtors = Constructable::getNumMoveConstructorCalls();
  EXPECT_TRUE(MoveCtors == 0 || MoveCtors == 2);
  size_t Dtors = Constructable::getNumDestructorCalls();
  EXPECT_TRUE(Dtors == 0 || Dtors == 2);
}

// Resize with fill value.
TYPED_TEST(CompactVectorTest, ResizeFillTest) {
  SCOPED_TRACE("ResizeFillTest");

  this->theVector.resize(3, Constructable(77));
  this->assertValuesInOrder(this->theVector, 3u, 77, 77, 77);
}

// Overflow past fixed size.
TYPED_TEST(CompactVectorTest, OverflowTest) {
  SCOPED_TRACE("OverflowTest");

  // Push more elements than the fixed size.
  this->makeSequence(this->theVector, 1, 10);

  // Test size and values.
  EXPECT_EQ(10u, this->theVector.size());
  for (int i = 0; i < 10; ++i) {
    EXPECT_EQ(i+1, this->theVector[i].getValue());
  }

  // Now resize back to fixed size.
  this->theVector.resize(1);

  this->assertValuesInOrder(this->theVector, 1u, 1);
}

// Iteration tests.
TYPED_TEST(CompactVectorTest, IterationTest) {
  this->makeSequence(this->theVector, 1, 2);

  // Forward Iteration
  typename TypeParam::iterator it = this->theVector.begin();
  EXPECT_TRUE(*it == this->theVector.front());
  EXPECT_TRUE(*it == this->theVector[0]);
  EXPECT_EQ(1, it->getValue());
  ++it;
  EXPECT_TRUE(*it == this->theVector[1]);
  EXPECT_TRUE(*it == this->theVector.back());
  EXPECT_EQ(2, it->getValue());
  ++it;
  EXPECT_TRUE(it == this->theVector.end());
  --it;
  EXPECT_TRUE(*it == this->theVector[1]);
  EXPECT_EQ(2, it->getValue());
  --it;
  EXPECT_TRUE(*it == this->theVector[0]);
  EXPECT_EQ(1, it->getValue());

  // Reverse Iteration
  typename TypeParam::reverse_iterator rit = this->theVector.rbegin();
  EXPECT_TRUE(*rit == this->theVector[1]);
  EXPECT_EQ(2, rit->getValue());
  ++rit;
  EXPECT_TRUE(*rit == this->theVector[0]);
  EXPECT_EQ(1, rit->getValue());
  ++rit;
  EXPECT_TRUE(rit == this->theVector.rend());
  --rit;
  EXPECT_TRUE(*rit == this->theVector[0]);
  EXPECT_EQ(1, rit->getValue());
  --rit;
  EXPECT_TRUE(*rit == this->theVector[1]);
  EXPECT_EQ(2, rit->getValue());
}

// Swap test.
TYPED_TEST(CompactVectorTest, SwapTest) {
  SCOPED_TRACE("SwapTest");

  this->makeSequence(this->theVector, 1, 2);
  std::swap(this->theVector, this->otherVector);

  this->assertEmpty(this->theVector);
  this->assertValuesInOrder(this->otherVector, 2u, 1, 2);
}

// Append test
TYPED_TEST(CompactVectorTest, AppendTest) {
  SCOPED_TRACE("AppendTest");

  this->makeSequence(this->otherVector, 2, 3);

  this->theVector.push_back(Constructable(1));
  this->theVector.insert(this->theVector.end(), this->otherVector.begin(), this->otherVector.end());

  this->assertValuesInOrder(this->theVector, 3u, 1, 2, 3);
}

// Append repeated test
TYPED_TEST(CompactVectorTest, AppendRepeatedTest) {
  SCOPED_TRACE("AppendRepeatedTest");

  this->theVector.push_back(Constructable(1));
  this->theVector.insert(this->theVector.end(), 2, Constructable(77));
  this->assertValuesInOrder(this->theVector, 3u, 1, 77, 77);
}

// Assign test
TYPED_TEST(CompactVectorTest, AssignTest) {
  SCOPED_TRACE("AssignTest");

  this->theVector.push_back(Constructable(1));
  this->theVector.assign(2, Constructable(77));
  this->assertValuesInOrder(this->theVector, 2u, 77, 77);
}

// Move-assign test
TYPED_TEST(CompactVectorTest, MoveAssignTest) {
  SCOPED_TRACE("MoveAssignTest");

  // Set up our vector with a single element, but enough capacity for 4.
  this->theVector.reserve(4);
  this->theVector.push_back(Constructable(1));

  // Set up the other vector with 2 elements.
  this->otherVector.push_back(Constructable(2));
  this->otherVector.push_back(Constructable(3));

  // Move-assign from the other vector.
  this->theVector = std::move(this->otherVector);

  // Make sure we have the right result.
  this->assertValuesInOrder(this->theVector, 2u, 2, 3);

  // Make sure the # of constructor/destructor calls line up. There
  // are two live objects after clearing the other vector.
  this->otherVector.clear();
  EXPECT_EQ(Constructable::getNumConstructorCalls()-2,
            Constructable::getNumDestructorCalls());

  // There shouldn't be any live objects any more.
  this->theVector.clear();
  EXPECT_EQ(Constructable::getNumConstructorCalls(),
            Constructable::getNumDestructorCalls());
}

// Erase a single element
TYPED_TEST(CompactVectorTest, EraseTest) {
  SCOPED_TRACE("EraseTest");

  this->makeSequence(this->theVector, 1, 3);
  this->theVector.erase(this->theVector.begin());
  this->assertValuesInOrder(this->theVector, 2u, 2, 3);
}

// Erase a range of elements
TYPED_TEST(CompactVectorTest, EraseRangeTest) {
  SCOPED_TRACE("EraseRangeTest");

  this->makeSequence(this->theVector, 1, 3);
  this->theVector.erase(this->theVector.begin(), this->theVector.begin() + 2);
  this->assertValuesInOrder(this->theVector, 1u, 3);
}

// Insert a single element.
TYPED_TEST(CompactVectorTest, InsertTest) {
  SCOPED_TRACE("InsertTest");

  this->makeSequence(this->theVector, 1, 3);
  typename TypeParam::iterator I =
    this->theVector.insert(this->theVector.begin() + 1, Constructable(77));
  EXPECT_EQ(this->theVector.begin() + 1, I);
  this->assertValuesInOrder(this->theVector, 4u, 1, 77, 2, 3);
}

// Insert a copy of a single element.
TYPED_TEST(CompactVectorTest, InsertCopy) {
  SCOPED_TRACE("InsertTest");

  this->makeSequence(this->theVector, 1, 3);
  Constructable C(77);
  typename TypeParam::iterator I =
      this->theVector.insert(this->theVector.begin() + 1, C);
  EXPECT_EQ(this->theVector.begin() + 1, I);
  this->assertValuesInOrder(this->theVector, 4u, 1, 77, 2, 3);
}

// Insert repeated elements.
TYPED_TEST(CompactVectorTest, InsertRepeatedTest) {
  SCOPED_TRACE("InsertRepeatedTest");

  this->makeSequence(this->theVector, 1, 4);
  Constructable::reset();
  auto I =
      this->theVector.insert(this->theVector.begin() + 1, 2, Constructable(16));
  // Move construct the top element into newly allocated space, and optionally
  // reallocate the whole buffer, move constructing into it.
  // FIXME: This is inefficient, we shouldn't move things into newly allocated
  // space, then move them up/around, there should only be 2 or 4 move
  // constructions here.
  EXPECT_TRUE(Constructable::getNumMoveConstructorCalls() == 2 ||
              Constructable::getNumMoveConstructorCalls() == 6);
  // Move assign the next two to shift them up and make a gap.
  EXPECT_EQ(1, Constructable::getNumMoveAssignmentCalls());
  // Copy construct the two new elements from the parameter.
  EXPECT_EQ(2, Constructable::getNumCopyAssignmentCalls());
  // All without any copy construction.
  EXPECT_EQ(0, Constructable::getNumCopyConstructorCalls());
  EXPECT_EQ(this->theVector.begin() + 1, I);
  this->assertValuesInOrder(this->theVector, 6u, 1, 16, 16, 2, 3, 4);
}


TYPED_TEST(CompactVectorTest, InsertRepeatedAtEndTest) {
  SCOPED_TRACE("InsertRepeatedTest");

  this->makeSequence(this->theVector, 1, 4);
  Constructable::reset();
  auto I = this->theVector.insert(this->theVector.end(), 2, Constructable(16));
  // Just copy construct them into newly allocated space
  EXPECT_EQ(2, Constructable::getNumCopyConstructorCalls());
  // Move everything across if reallocation is needed.
  EXPECT_TRUE(Constructable::getNumMoveConstructorCalls() == 0 ||
              Constructable::getNumMoveConstructorCalls() == 4);
  // Without ever moving or copying anything else.
  EXPECT_EQ(0, Constructable::getNumCopyAssignmentCalls());
  EXPECT_EQ(0, Constructable::getNumMoveAssignmentCalls());

  EXPECT_EQ(this->theVector.begin() + 4, I);
  this->assertValuesInOrder(this->theVector, 6u, 1, 2, 3, 4, 16, 16);
}

TYPED_TEST(CompactVectorTest, InsertRepeatedEmptyTest) {
  SCOPED_TRACE("InsertRepeatedTest");

  this->makeSequence(this->theVector, 10, 15);

  // Empty insert.
  EXPECT_EQ(this->theVector.end(),
            this->theVector.insert(this->theVector.end(),
                                   0, Constructable(42)));
  EXPECT_EQ(this->theVector.begin() + 1,
            this->theVector.insert(this->theVector.begin() + 1,
                                   0, Constructable(42)));
}

// Insert range.
TYPED_TEST(CompactVectorTest, InsertRangeTest) {
  SCOPED_TRACE("InsertRangeTest");

  Constructable Arr[3] =
    { Constructable(77), Constructable(77), Constructable(77) };

  this->makeSequence(this->theVector, 1, 3);
  Constructable::reset();
  auto I = this->theVector.insert(this->theVector.begin() + 1, Arr, Arr + 3);
  // Move construct the top 3 elements into newly allocated space.
  // Possibly move the whole sequence into new space first.
  // FIXME: This is inefficient, we shouldn't move things into newly allocated
  // space, then move them up/around, there should only be 2 or 3 move
  // constructions here.
  EXPECT_TRUE(Constructable::getNumMoveConstructorCalls() == 2 ||
              Constructable::getNumMoveConstructorCalls() == 5);
  // Copy assign the lower 2 new elements into existing space.
  EXPECT_EQ(2, Constructable::getNumCopyAssignmentCalls());
  // Copy construct the third element into newly allocated space.
  EXPECT_EQ(1, Constructable::getNumCopyConstructorCalls());
  EXPECT_EQ(this->theVector.begin() + 1, I);
  this->assertValuesInOrder(this->theVector, 6u, 1, 77, 77, 77, 2, 3);
}


TYPED_TEST(CompactVectorTest, InsertRangeAtEndTest) {
  SCOPED_TRACE("InsertRangeTest");

  Constructable Arr[3] =
    { Constructable(77), Constructable(77), Constructable(77) };

  this->makeSequence(this->theVector, 1, 3);

  // Insert at end.
  Constructable::reset();
  auto I = this->theVector.insert(this->theVector.end(), Arr, Arr+3);
  // Copy construct the 3 elements into new space at the top.
  EXPECT_EQ(3, Constructable::getNumCopyConstructorCalls());
  // Don't copy/move anything else.
  EXPECT_EQ(0, Constructable::getNumCopyAssignmentCalls());
  // Reallocation might occur, causing all elements to be moved into the new
  // buffer.
  EXPECT_TRUE(Constructable::getNumMoveConstructorCalls() == 0 ||
              Constructable::getNumMoveConstructorCalls() == 3);
  EXPECT_EQ(0, Constructable::getNumMoveAssignmentCalls());
  EXPECT_EQ(this->theVector.begin() + 3, I);
  this->assertValuesInOrder(this->theVector, 6u,
                            1, 2, 3, 77, 77, 77);
}

TYPED_TEST(CompactVectorTest, InsertEmptyRangeTest) {
  SCOPED_TRACE("InsertRangeTest");

  this->makeSequence(this->theVector, 1, 3);

  // Empty insert.
  EXPECT_EQ(this->theVector.end(),
            this->theVector.insert(this->theVector.end(),
                                   this->theVector.begin(),
                                   this->theVector.begin()));
  EXPECT_EQ(this->theVector.begin() + 1,
            this->theVector.insert(this->theVector.begin() + 1,
                                   this->theVector.begin(),
                                   this->theVector.begin()));
}

// Comparison tests.
TYPED_TEST(CompactVectorTest, ComparisonTest) {
  SCOPED_TRACE("ComparisonTest");

  this->makeSequence(this->theVector, 1, 3);
  this->makeSequence(this->otherVector, 1, 3);

  EXPECT_TRUE(this->theVector == this->otherVector);
  EXPECT_FALSE(this->theVector != this->otherVector);

  this->otherVector.clear();
  this->makeSequence(this->otherVector, 2, 4);

  EXPECT_FALSE(this->theVector == this->otherVector);
  EXPECT_TRUE(this->theVector != this->otherVector);
}

// Constant vector tests.
TYPED_TEST(CompactVectorTest, ConstVectorTest) {
  const TypeParam constVector;

  EXPECT_EQ(0u, constVector.size());
  EXPECT_TRUE(constVector.empty());
  EXPECT_TRUE(constVector.begin() == constVector.end());
}

// Direct array access.
TYPED_TEST(CompactVectorTest, DirectVectorTest) {
  EXPECT_EQ(0u, this->theVector.size());
  this->theVector.reserve(4);
  EXPECT_LE(4u, this->theVector.capacity());
  EXPECT_EQ(0, Constructable::getNumConstructorCalls());
  this->theVector.push_back(1);
  this->theVector.push_back(2);
  this->theVector.push_back(3);
  this->theVector.push_back(4);
  EXPECT_EQ(4u, this->theVector.size());
  EXPECT_EQ(8, Constructable::getNumConstructorCalls());
  EXPECT_EQ(1, this->theVector[0].getValue());
  EXPECT_EQ(2, this->theVector[1].getValue());
  EXPECT_EQ(3, this->theVector[2].getValue());
  EXPECT_EQ(4, this->theVector[3].getValue());
}

TYPED_TEST(CompactVectorTest, IteratorTest) {
  std::list<int> L;
  this->theVector.insert(this->theVector.end(), L.begin(), L.end());
}

template <typename InvalidType> class DualCompactVectorsTest;

template <typename VectorT1, typename VectorT2>
class DualCompactVectorsTest<std::pair<VectorT1, VectorT2>> : public CompactVectorTestBase {
protected:
  VectorT1 theVector;
  VectorT2 otherVector;

  template <typename T, size_t N>
  static size_t NumBuiltinElts(const TCompactVector<T, N>&) { return N; }
};

using DualCompactVectorTestTypes = ::testing::Types<
    // Small mode -> Small mode.
    std::pair<TCompactVector<Constructable, 4>, TCompactVector<Constructable, 4>>,
    // Small mode -> Big mode.
    std::pair<TCompactVector<Constructable, 4>, TCompactVector<Constructable, 2>>,
    // Big mode -> Small mode.
    std::pair<TCompactVector<Constructable, 2>, TCompactVector<Constructable, 4>>,
    // Big mode -> Big mode.
    std::pair<TCompactVector<Constructable, 2>, TCompactVector<Constructable, 2>>
  >;

TYPED_TEST_SUITE(DualCompactVectorsTest, DualCompactVectorTestTypes);

TYPED_TEST(DualCompactVectorsTest, MoveAssignment) {
  SCOPED_TRACE("MoveAssignTest-DualVectorTypes");

  // Set up our vector with four elements.
  for (unsigned I = 0; I < 4; ++I)
    this->otherVector.push_back(Constructable(I));

  const Constructable *OrigDataPtr = this->otherVector.data();

  // Move-assign from the other vector.
  this->theVector =
    std::move(this->otherVector);

  // Make sure we have the right result.
  this->assertValuesInOrder(this->theVector, 4u, 0, 1, 2, 3);

  // Make sure the # of constructor/destructor calls line up. There
  // are two live objects after clearing the other vector.
  this->otherVector.clear();
  EXPECT_EQ(Constructable::getNumConstructorCalls()-4,
            Constructable::getNumDestructorCalls());

  // If the source vector (otherVector) was in small-mode, assert that we just
  // moved the data pointer over.
  EXPECT_TRUE(this->NumBuiltinElts(this->otherVector) == 4 ||
              this->theVector.data() == OrigDataPtr);

  // There shouldn't be any live objects any more.
  this->theVector.clear();
  EXPECT_EQ(Constructable::getNumConstructorCalls(),
            Constructable::getNumDestructorCalls());

  // We shouldn't have copied anything in this whole process.
  EXPECT_EQ(Constructable::getNumCopyConstructorCalls(), 0);
}

struct notassignable {
  int &x;
  notassignable(int &x) : x(x) {}
};

TEST(CompactVectorCustomTest, NoAssignTest) {
  int x = 0;
  TCompactVector<notassignable, 2> vec;
  vec.push_back(notassignable(x));
  x = 42;
  EXPECT_EQ(42, vec.back().x);
}

struct MovedFrom {
  bool hasValue;
  MovedFrom() : hasValue(true) {
  }
  MovedFrom(MovedFrom&& m) : hasValue(m.hasValue) {
    m.hasValue = false;
  }
  MovedFrom &operator=(MovedFrom&& m) {
    hasValue = m.hasValue;
    m.hasValue = false;
    return *this;
  }
};

TEST(CompactVectorTest, MidInsert) {
  TCompactVector<MovedFrom, 3> v;
  v.push_back(MovedFrom());
  v.insert(v.begin(), MovedFrom());
  for (MovedFrom &m : v)
    EXPECT_TRUE(m.hasValue);
}

enum EmplaceableArgState {
  EAS_Defaulted,
  EAS_Arg,
  EAS_LValue,
  EAS_RValue,
  EAS_Failure
};
template <int I> struct EmplaceableArg {
  EmplaceableArgState State;
  EmplaceableArg() : State(EAS_Defaulted) {}
  EmplaceableArg(EmplaceableArg &&X)
      : State(X.State == EAS_Arg ? EAS_RValue : EAS_Failure) {}
  EmplaceableArg(EmplaceableArg &X)
      : State(X.State == EAS_Arg ? EAS_LValue : EAS_Failure) {}

  explicit EmplaceableArg(bool) : State(EAS_Arg) {}

private:
  EmplaceableArg &operator=(EmplaceableArg &&) = delete;
  EmplaceableArg &operator=(const EmplaceableArg &) = delete;
};

enum EmplaceableState { ES_Emplaced, ES_Moved };
struct Emplaceable {
  EmplaceableArg<0> A0;
  EmplaceableArg<1> A1;
  EmplaceableArg<2> A2;
  EmplaceableArg<3> A3;
  EmplaceableState State;

  Emplaceable() : State(ES_Emplaced) {}

  template <class A0Ty>
  explicit Emplaceable(A0Ty &&A0)
      : A0(std::forward<A0Ty>(A0)), State(ES_Emplaced) {}

  template <class A0Ty, class A1Ty>
  Emplaceable(A0Ty &&A0, A1Ty &&A1)
      : A0(std::forward<A0Ty>(A0)), A1(std::forward<A1Ty>(A1)),
        State(ES_Emplaced) {}

  template <class A0Ty, class A1Ty, class A2Ty>
  Emplaceable(A0Ty &&A0, A1Ty &&A1, A2Ty &&A2)
      : A0(std::forward<A0Ty>(A0)), A1(std::forward<A1Ty>(A1)),
        A2(std::forward<A2Ty>(A2)), State(ES_Emplaced) {}

  template <class A0Ty, class A1Ty, class A2Ty, class A3Ty>
  Emplaceable(A0Ty &&A0, A1Ty &&A1, A2Ty &&A2, A3Ty &&A3)
      : A0(std::forward<A0Ty>(A0)), A1(std::forward<A1Ty>(A1)),
        A2(std::forward<A2Ty>(A2)), A3(std::forward<A3Ty>(A3)),
        State(ES_Emplaced) {}

  Emplaceable(Emplaceable &&) : State(ES_Moved) {}
  Emplaceable &operator=(Emplaceable &&) {
    State = ES_Moved;
    return *this;
  }

private:
  Emplaceable(const Emplaceable &) = delete;
  Emplaceable &operator=(const Emplaceable &) = delete;
};

TEST(CompactVectorTest, EmplaceBack) {
  EmplaceableArg<0> A0(true);
  EmplaceableArg<1> A1(true);
  EmplaceableArg<2> A2(true);
  EmplaceableArg<3> A3(true);
  {
    TCompactVector<Emplaceable, 3> V;
    V.emplace_back();
    EXPECT_TRUE(V.size() == 1);
    EXPECT_TRUE(V.back().State == ES_Emplaced);
    EXPECT_TRUE(V.back().A0.State == EAS_Defaulted);
    EXPECT_TRUE(V.back().A1.State == EAS_Defaulted);
    EXPECT_TRUE(V.back().A2.State == EAS_Defaulted);
    EXPECT_TRUE(V.back().A3.State == EAS_Defaulted);
  }
  {
    TCompactVector<Emplaceable, 3> V;
    V.emplace_back(std::move(A0));
    EXPECT_TRUE(V.size() == 1);
    EXPECT_TRUE(V.back().State == ES_Emplaced);
    EXPECT_TRUE(V.back().A0.State == EAS_RValue);
    EXPECT_TRUE(V.back().A1.State == EAS_Defaulted);
    EXPECT_TRUE(V.back().A2.State == EAS_Defaulted);
    EXPECT_TRUE(V.back().A3.State == EAS_Defaulted);
  }
  {
    TCompactVector<Emplaceable, 3> V;
    V.emplace_back(A0);
    EXPECT_TRUE(V.size() == 1);
    EXPECT_TRUE(V.back().State == ES_Emplaced);
    EXPECT_TRUE(V.back().A0.State == EAS_LValue);
    EXPECT_TRUE(V.back().A1.State == EAS_Defaulted);
    EXPECT_TRUE(V.back().A2.State == EAS_Defaulted);
    EXPECT_TRUE(V.back().A3.State == EAS_Defaulted);
  }
  {
    TCompactVector<Emplaceable, 3> V;
    V.emplace_back(A0, A1);
    EXPECT_TRUE(V.size() == 1);
    EXPECT_TRUE(V.back().State == ES_Emplaced);
    EXPECT_TRUE(V.back().A0.State == EAS_LValue);
    EXPECT_TRUE(V.back().A1.State == EAS_LValue);
    EXPECT_TRUE(V.back().A2.State == EAS_Defaulted);
    EXPECT_TRUE(V.back().A3.State == EAS_Defaulted);
  }
  {
    TCompactVector<Emplaceable, 3> V;
    V.emplace_back(std::move(A0), std::move(A1));
    EXPECT_TRUE(V.size() == 1);
    EXPECT_TRUE(V.back().State == ES_Emplaced);
    EXPECT_TRUE(V.back().A0.State == EAS_RValue);
    EXPECT_TRUE(V.back().A1.State == EAS_RValue);
    EXPECT_TRUE(V.back().A2.State == EAS_Defaulted);
    EXPECT_TRUE(V.back().A3.State == EAS_Defaulted);
  }
  {
    TCompactVector<Emplaceable, 3> V;
    V.emplace_back(std::move(A0), A1, std::move(A2), A3);
    EXPECT_TRUE(V.size() == 1);
    EXPECT_TRUE(V.back().State == ES_Emplaced);
    EXPECT_TRUE(V.back().A0.State == EAS_RValue);
    EXPECT_TRUE(V.back().A1.State == EAS_LValue);
    EXPECT_TRUE(V.back().A2.State == EAS_RValue);
    EXPECT_TRUE(V.back().A3.State == EAS_LValue);
  }
  {
    TCompactVector<int, 1> V;
    V.emplace_back();
    V.emplace_back(42);
    EXPECT_EQ(2U, V.size());
    EXPECT_EQ(0, V[0]);
    EXPECT_EQ(42, V[1]);
  }
}

TEST(CompactVectorTest, Emplace) {
  EmplaceableArg<0> A0(true);
  EmplaceableArg<1> A1(true);
  EmplaceableArg<2> A2(true);
  EmplaceableArg<3> A3(true);
  {
    TCompactVector<Emplaceable, 3> V;
    V.emplace(V.end());
    EXPECT_TRUE(V.size() == 1);
    EXPECT_TRUE(V.back().State == ES_Emplaced);
    EXPECT_TRUE(V.back().A0.State == EAS_Defaulted);
    EXPECT_TRUE(V.back().A1.State == EAS_Defaulted);
    EXPECT_TRUE(V.back().A2.State == EAS_Defaulted);
    EXPECT_TRUE(V.back().A3.State == EAS_Defaulted);
  }
  {
    TCompactVector<Emplaceable, 3> V;
    V.emplace(V.end(), std::move(A0));
    EXPECT_TRUE(V.size() == 1);
    EXPECT_TRUE(V.back().State == ES_Emplaced);
    EXPECT_TRUE(V.back().A0.State == EAS_RValue);
    EXPECT_TRUE(V.back().A1.State == EAS_Defaulted);
    EXPECT_TRUE(V.back().A2.State == EAS_Defaulted);
    EXPECT_TRUE(V.back().A3.State == EAS_Defaulted);
  }
  {
    TCompactVector<Emplaceable, 3> V;
    V.emplace(V.end(), A0);
    EXPECT_TRUE(V.size() == 1);
    EXPECT_TRUE(V.back().State == ES_Emplaced);
    EXPECT_TRUE(V.back().A0.State == EAS_LValue);
    EXPECT_TRUE(V.back().A1.State == EAS_Defaulted);
    EXPECT_TRUE(V.back().A2.State == EAS_Defaulted);
    EXPECT_TRUE(V.back().A3.State == EAS_Defaulted);
  }
  {
    TCompactVector<Emplaceable, 3> V;
    V.emplace(V.end(), A0, A1);
    EXPECT_TRUE(V.size() == 1);
    EXPECT_TRUE(V.back().State == ES_Emplaced);
    EXPECT_TRUE(V.back().A0.State == EAS_LValue);
    EXPECT_TRUE(V.back().A1.State == EAS_LValue);
    EXPECT_TRUE(V.back().A2.State == EAS_Defaulted);
    EXPECT_TRUE(V.back().A3.State == EAS_Defaulted);
  }
  {
    TCompactVector<Emplaceable, 3> V;
    V.emplace(V.end(), std::move(A0), std::move(A1));
    EXPECT_TRUE(V.size() == 1);
    EXPECT_TRUE(V.back().State == ES_Emplaced);
    EXPECT_TRUE(V.back().A0.State == EAS_RValue);
    EXPECT_TRUE(V.back().A1.State == EAS_RValue);
    EXPECT_TRUE(V.back().A2.State == EAS_Defaulted);
    EXPECT_TRUE(V.back().A3.State == EAS_Defaulted);
  }
  {
    TCompactVector<Emplaceable, 3> V;
    V.emplace(V.end(), std::move(A0), A1, std::move(A2), A3);
    EXPECT_TRUE(V.size() == 1);
    EXPECT_TRUE(V.back().State == ES_Emplaced);
    EXPECT_TRUE(V.back().A0.State == EAS_RValue);
    EXPECT_TRUE(V.back().A1.State == EAS_LValue);
    EXPECT_TRUE(V.back().A2.State == EAS_RValue);
    EXPECT_TRUE(V.back().A3.State == EAS_LValue);
  }
  {
    TCompactVector<int, 1> V;
    V.emplace_back(42);
    V.emplace(V.begin(), 0);
    EXPECT_EQ(2U, V.size());
    EXPECT_EQ(0, V[0]);
    EXPECT_EQ(42, V[1]);
  }
}

template <class T, size_t N>
class TStubArray
{
public:
    TStubArray(const TCompactVector<T, N>& vector)
        : Vector_(vector)
    { }

    bool equals(std::initializer_list<T> list)
    {
        return std::equal(Vector_.begin(), Vector_.end(), list.begin());
    }

    TCompactVector<T, N> Vector_;
};

template <typename T, size_t N>
TStubArray<T, N> makeArrayRef(const TCompactVector<T, N>& vector)
{
    return TStubArray<T, N>(vector);
}

TEST(CompactVectorTest, InitializerList) {
  TCompactVector<int, 2> V1 = {};
  EXPECT_TRUE(V1.empty());
  V1 = {0, 0};
  EXPECT_TRUE(makeArrayRef(V1).equals({0, 0}));
  V1 = {-1, -1};
  EXPECT_TRUE(makeArrayRef(V1).equals({-1, -1}));

  TCompactVector<int, 2> V2 = {1, 2, 3, 4};
  EXPECT_TRUE(makeArrayRef(V2).equals({1, 2, 3, 4}));
  V2.assign({4});
  EXPECT_TRUE(makeArrayRef(V2).equals({4}));
  V2.insert(V2.end(), {3, 2});
  EXPECT_TRUE(makeArrayRef(V2).equals({4, 3, 2}));
  V2.insert(V2.begin() + 1, 5);
  EXPECT_TRUE(makeArrayRef(V2).equals({4, 5, 3, 2}));
}

TEST(CompactVectorTest, AssignToShorter) {
  TCompactVector<std::string, 4> lhs;
  TCompactVector<std::string, 4> rhs;
  rhs.emplace_back("foo");
  lhs = rhs;
  EXPECT_EQ(1U, lhs.size());
  EXPECT_EQ("foo", lhs[0]);
}

TEST(CompactVectorTest, AssignToLonger) {
  TCompactVector<std::string, 4> lhs;
  lhs.emplace_back("bar");
  lhs.emplace_back("baz");
  TCompactVector<std::string, 4> rhs;
  rhs.emplace_back("foo");
  lhs = rhs;
  EXPECT_EQ(1U, lhs.size());
  EXPECT_EQ("foo", lhs[0]);
}

TEST(CompactVectorTest, ZeroPaddingOnHeapMeta) {
  TCompactVector<uint8_t, 6> vector;
  std::vector<uint8_t> expected;
  for (int i = 0; i < 10; ++i) {
    vector.push_back(i);
    expected.push_back(i);

    ASSERT_THAT(vector, ::testing::ElementsAreArray(expected));
  }
}

////////////////////////////////////////////////////////////////////////////////

} // namespace
} // namespace NYT
