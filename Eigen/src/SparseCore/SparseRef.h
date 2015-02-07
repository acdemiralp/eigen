// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2015 Gael Guennebaud <gael.guennebaud@inria.fr>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef EIGEN_SPARSE_REF_H
#define EIGEN_SPARSE_REF_H

namespace Eigen {

namespace internal {

template<typename Derived> class SparseRefBase;
  
template<typename MatScalar, int MatOptions, typename MatIndex, int _Options, typename _StrideType>
struct traits<Ref<SparseMatrix<MatScalar,MatOptions,MatIndex>, _Options, _StrideType> >
  : public traits<SparseMatrix<MatScalar,MatOptions,MatIndex> >
{
  typedef SparseMatrix<MatScalar,MatOptions,MatIndex> PlainObjectType;
  enum {
    Options = _Options,
    Flags = traits<MappedSparseMatrix<MatScalar,MatOptions,MatIndex> >::Flags | CompressedAccessBit | NestByRefBit
  };

  template<typename Derived> struct match {
    enum {
      StorageOrderMatch = PlainObjectType::IsVectorAtCompileTime || Derived::IsVectorAtCompileTime || ((PlainObjectType::Flags&RowMajorBit)==(Derived::Flags&RowMajorBit)),
      MatchAtCompileTime = (Derived::Flags&CompressedAccessBit) && StorageOrderMatch
    };
    typedef typename internal::conditional<MatchAtCompileTime,internal::true_type,internal::false_type>::type type;
  };
  
};

template<typename MatScalar, int MatOptions, typename MatIndex, int _Options, typename _StrideType>
struct traits<Ref<const SparseMatrix<MatScalar,MatOptions,MatIndex>, _Options, _StrideType> >
  : public traits<Ref<SparseMatrix<MatScalar,MatOptions,MatIndex>, _Options, _StrideType> >
{
  enum {
    Flags = (traits<MappedSparseMatrix<MatScalar,MatOptions,MatIndex> >::Flags | CompressedAccessBit | NestByRefBit) & ~LvalueBit
  };
};
  
template<typename Derived>
struct traits<SparseRefBase<Derived> > : public traits<Derived> {};

template<typename Derived> class SparseRefBase
//  : public MappedSparseMatrix<MatScalar,MatOptions,MatIndex>
  : public SparseMapBase<Derived>
{
//   typedef typename internal::traits<Derived>::PlainObjectType PlainObjectType;

public:

  typedef SparseMapBase<Derived> Base;
  _EIGEN_SPARSE_PUBLIC_INTERFACE(SparseRefBase)

  SparseRefBase()
    : Base(RowsAtCompileTime==Dynamic?0:RowsAtCompileTime,ColsAtCompileTime==Dynamic?0:ColsAtCompileTime, 0, 0, 0, 0, 0)
  {}
  
  EIGEN_INHERIT_ASSIGNMENT_OPERATORS(SparseRefBase)

protected:


  template<typename Expression>
  void construct(Expression& expr)
  {
    ::new (static_cast<Base*>(this)) Base(expr.rows(), expr.cols(), expr.nonZeros(), expr.outerIndexPtr(), expr.innerIndexPtr(), expr.valuePtr(), expr.innerNonZeroPtr());
  }
};

} // namespace internal

template<typename MatScalar, int MatOptions, typename MatIndex, int Options, typename StrideType>
class Ref<SparseMatrix<MatScalar,MatOptions,MatIndex>, Options, StrideType >
  : public internal::SparseRefBase<Ref<SparseMatrix<MatScalar,MatOptions,MatIndex>, Options, StrideType > >
{
    typedef SparseMatrix<MatScalar,MatOptions,MatIndex> PlainObjectType;
    typedef internal::traits<Ref> Traits;
    template<int OtherOptions>
    inline Ref(const SparseMatrix<MatScalar,OtherOptions,MatIndex>& expr);
    template<int OtherOptions>
    inline Ref(const MappedSparseMatrix<MatScalar,OtherOptions,MatIndex>& expr);
  public:

    typedef internal::SparseRefBase<Ref> Base;
    _EIGEN_SPARSE_PUBLIC_INTERFACE(Ref)


    #ifndef EIGEN_PARSED_BY_DOXYGEN
    template<int OtherOptions>
    inline Ref(SparseMatrix<MatScalar,OtherOptions,MatIndex>& expr)
    {
      EIGEN_STATIC_ASSERT(bool(Traits::template match<SparseMatrix<MatScalar,OtherOptions,MatIndex> >::MatchAtCompileTime), STORAGE_LAYOUT_DOES_NOT_MATCH);
      Base::construct(expr.derived());
    }
    
    template<int OtherOptions>
    inline Ref(MappedSparseMatrix<MatScalar,OtherOptions,MatIndex>& expr)
    {
      EIGEN_STATIC_ASSERT(bool(Traits::template match<SparseMatrix<MatScalar,OtherOptions,MatIndex> >::MatchAtCompileTime), STORAGE_LAYOUT_DOES_NOT_MATCH);
      Base::construct(expr.derived());
    }
    
    template<typename Derived>
    inline Ref(const SparseCompressedBase<Derived>& expr)
    #else
    template<typename Derived>
    inline Ref(SparseCompressedBase<Derived>& expr)
    #endif
    {
      EIGEN_STATIC_ASSERT(bool(internal::is_lvalue<Derived>::value), THIS_EXPRESSION_IS_NOT_A_LVALUE__IT_IS_READ_ONLY);
      EIGEN_STATIC_ASSERT(bool(Traits::template match<Derived>::MatchAtCompileTime), STORAGE_LAYOUT_DOES_NOT_MATCH);
      Base::construct(expr.const_cast_derived());
    }

    EIGEN_INHERIT_ASSIGNMENT_OPERATORS(Ref)

};

// this is the const ref version
template<typename MatScalar, int MatOptions, typename MatIndex, int Options, typename StrideType>
class Ref<const SparseMatrix<MatScalar,MatOptions,MatIndex>, Options, StrideType>
  : public internal::SparseRefBase<Ref<const SparseMatrix<MatScalar,MatOptions,MatIndex>, Options, StrideType> >
{
    typedef SparseMatrix<MatScalar,MatOptions,MatIndex> TPlainObjectType;
    typedef internal::traits<Ref> Traits;
  public:

    typedef internal::SparseRefBase<Ref> Base;
    _EIGEN_SPARSE_PUBLIC_INTERFACE(Ref)

    template<typename Derived>
    inline Ref(const SparseMatrixBase<Derived>& expr)
    {
      construct(expr.derived(), typename Traits::template match<Derived>::type());
    }

    inline Ref(const Ref& other) : Base(other) {
      // copy constructor shall not copy the m_object, to avoid unnecessary malloc and copy
    }

    template<typename OtherRef>
    inline Ref(const RefBase<OtherRef>& other) {
      construct(other.derived(), typename Traits::template match<OtherRef>::type());
    }

  protected:

    template<typename Expression>
    void construct(const Expression& expr,internal::true_type)
    {
      Base::construct(expr);
    }

    template<typename Expression>
    void construct(const Expression& expr, internal::false_type)
    {
      m_object = expr;
      Base::construct(m_object);
    }

  protected:
    TPlainObjectType m_object;
};


namespace internal {

template<typename MatScalar, int MatOptions, typename MatIndex, int Options, typename StrideType>
struct evaluator<Ref<SparseMatrix<MatScalar,MatOptions,MatIndex>, Options, StrideType> >
  : evaluator<SparseCompressedBase<Ref<SparseMatrix<MatScalar,MatOptions,MatIndex>, Options, StrideType> > >
{
  typedef evaluator<SparseCompressedBase<Ref<SparseMatrix<MatScalar,MatOptions,MatIndex>, Options, StrideType> > > Base;
  typedef Ref<SparseMatrix<MatScalar,MatOptions,MatIndex>, Options, StrideType> XprType;  
  evaluator() : Base() {}
  explicit evaluator(const XprType &mat) : Base(mat) {}
};

template<typename MatScalar, int MatOptions, typename MatIndex, int Options, typename StrideType>
struct evaluator<Ref<const SparseMatrix<MatScalar,MatOptions,MatIndex>, Options, StrideType> >
  : evaluator<SparseCompressedBase<Ref<const SparseMatrix<MatScalar,MatOptions,MatIndex>, Options, StrideType> > >
{
  typedef evaluator<SparseCompressedBase<Ref<const SparseMatrix<MatScalar,MatOptions,MatIndex>, Options, StrideType> > > Base;
  typedef Ref<const SparseMatrix<MatScalar,MatOptions,MatIndex>, Options, StrideType> XprType;  
  evaluator() : Base() {}
  explicit evaluator(const XprType &mat) : Base(mat) {}
};

}

} // end namespace Eigen

#endif // EIGEN_SPARSE_REF_H