// Support for interoperability between Boost.System and <system_error>
//
// Copyright 2018 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See library home page at http://www.boost.org/libs/system

#include <system_error>
#include <map>
#include <memory>
#include <mutex>

//

namespace cppmsboost
{

namespace system
{

namespace detail
{

class BOOST_SYMBOL_VISIBLE std_category: public std::error_category
{
private:

    cppmsboost::system::error_category const * pc_;

public:

    explicit std_category( cppmsboost::system::error_category const * pc, unsigned id ): pc_( pc )
    {
        if( id != 0 )
        {
#if defined(_MSC_VER) && defined(_CPPLIB_VER) && _MSC_VER >= 1900 && _MSC_VER < 2000

            // Poking into the protected _Addr member of std::error_category
            // is not a particularly good programming practice, but what can
            // you do

            _Addr = id;

#endif
        }
    }

    virtual const char * name() const BOOST_NOEXCEPT
    {
        return pc_->name();
    }

    virtual std::string message( int ev ) const
    {
        return pc_->message( ev );
    }

    virtual std::error_condition default_error_condition( int ev ) const BOOST_NOEXCEPT
    {
        return pc_->default_error_condition( ev );
    }

    virtual bool equivalent( int code, const std::error_condition & condition ) const BOOST_NOEXCEPT;
    virtual bool equivalent( const std::error_code & code, int condition ) const BOOST_NOEXCEPT;
};

#if !defined(__SUNPRO_CC) // trailing __global is not supported
inline std::error_category const & to_std_category( cppmsboost::system::error_category const & cat ) BOOST_SYMBOL_VISIBLE;
#endif

struct cat_ptr_less
{
    bool operator()( cppmsboost::system::error_category const * p1, cppmsboost::system::error_category const * p2 ) const BOOST_NOEXCEPT
    {
        return *p1 < *p2;
    }
};

inline std::error_category const & to_std_category( cppmsboost::system::error_category const & cat )
{
    if( cat == cppmsboost::system::system_category() )
    {
        static const std_category system_instance( &cat, 0x1F4D7 );
        return system_instance;
    }
    else if( cat == cppmsboost::system::generic_category() )
    {
        static const std_category generic_instance( &cat, 0x1F4D3 );
        return generic_instance;
    }
    else
    {
        typedef std::map< cppmsboost::system::error_category const *, std::unique_ptr<std_category>, cat_ptr_less > map_type;

        static map_type map_;
        static std::mutex map_mx_;

        std::lock_guard<std::mutex> guard( map_mx_ );

        map_type::iterator i = map_.find( &cat );

        if( i == map_.end() )
        {
            std::unique_ptr<std_category> p( new std_category( &cat, 0 ) );

            std::pair<map_type::iterator, bool> r = map_.insert( map_type::value_type( &cat, std::move( p ) ) );

            i = r.first;
        }

        return *i->second;
    }
}

inline bool std_category::equivalent( int code, const std::error_condition & condition ) const BOOST_NOEXCEPT
{
    if( condition.category() == *this )
    {
        cppmsboost::system::error_condition bn( condition.value(), *pc_ );
        return pc_->equivalent( code, bn );
    }
    else if( condition.category() == std::generic_category() || condition.category() == cppmsboost::system::generic_category() )
    {
        cppmsboost::system::error_condition bn( condition.value(), cppmsboost::system::generic_category() );
        return pc_->equivalent( code, bn );
    }

#ifndef BOOST_NO_RTTI

    else if( std_category const* pc2 = dynamic_cast< std_category const* >( &condition.category() ) )
    {
        cppmsboost::system::error_condition bn( condition.value(), *pc2->pc_ );
        return pc_->equivalent( code, bn );
    }

#endif

    else
    {
        return default_error_condition( code ) == condition;
    }
}

inline bool std_category::equivalent( const std::error_code & code, int condition ) const BOOST_NOEXCEPT
{
    if( code.category() == *this )
    {
        cppmsboost::system::error_code bc( code.value(), *pc_ );
        return pc_->equivalent( bc, condition );
    }
    else if( code.category() == std::generic_category() || code.category() == cppmsboost::system::generic_category() )
    {
        cppmsboost::system::error_code bc( code.value(), cppmsboost::system::generic_category() );
        return pc_->equivalent( bc, condition );
    }

#ifndef BOOST_NO_RTTI

    else if( std_category const* pc2 = dynamic_cast< std_category const* >( &code.category() ) )
    {
        cppmsboost::system::error_code bc( code.value(), *pc2->pc_ );
        return pc_->equivalent( bc, condition );
    }
#endif

    else if( *pc_ == cppmsboost::system::generic_category() )
    {
        return std::generic_category().equivalent( code, condition );
    }
    else
    {
        return false;
    }
}

} // namespace detail

} // namespace system

} // namespace cppmsboost