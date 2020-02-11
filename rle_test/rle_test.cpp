// rle_test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>


#include <cassert>
#include <deque>
#include <optional>
#include <string>
#include <vector>

// encoded rle test data
using rle_data = std::vector< char >;
auto const g_data = rle_data{ 4, 2, -3, 5, 1, 2, 5, 9 };

// sample iterator, indexed into both descriptor array and data array
struct iterator
{
    size_t desc_idx{}; // index into descriptor array
    size_t data_idx{}; // index into data array
    char counter{};    // run counter -- e.g. a run can be  {4, 2}  or  {-3, 5, 1, 2}
};

// unoptimized utility class to wrap data and utility methods
class render_row
{
public:
    explicit render_row( rle_data const& data, int const width )
    {
        // split rle data into descriptor and data
        rle_desc_.reserve( data.size() / 2 ); // approximation
        rle_data_.reserve( data.size() / 2 ); // approximation
        for ( auto i = size_t{}; i < data.size(); ++i )
        {
            auto const desc = data[ i ];
            assert( desc != 0 );
            rle_desc_.push_back( desc );
            if ( desc > 0 ) {
                assert( ( i + 1 ) < data.size() ); // invalid data
                rle_data_.push_back( data[ ++i ] );
            }
            else
            {
                for ( auto const end = i - desc; i < end; ++i ) {
                    assert( ( i + 1 ) < data.size() ); // invalid data
                    rle_data_.push_back( data[ i + 1 ] );
                }
            }
        }

        // fill initial render buffer to window width
        for ( auto i = 0; i < width; ++i )
        {
            push_render_right();
            if ( !advance_right( end_ ) ) break;
        }
        render_offset_.clear();
    }

    // advance iterators and push to render buffer
    bool advance_right()
    {
        if ( push_render_right() )
        {
            advance_right( begin_ );
            advance_right( end_ );
            render_data_.pop_front();
            return true;
        }
        return false;
    }
    bool advance_left()
    {
        if ( advance_left( begin_ ) )
        {
            push_render_left();
            advance_left( end_ );
            render_data_.pop_back();
            return true;
        }
        return false;
    }

    void render() const
    {
        std::cout << render_offset_;
        for ( auto const& v : render_data_ )
        {
            std::cout << static_cast< int >( v );
        }
        std::cout << std::endl;
    }

private:
    std::vector< char > rle_desc_{}; // descriptor array
    std::vector< char > rle_data_{}; // data array
    iterator begin_{};
    iterator end_{};

    using render_data = std::deque< char >;
    render_data render_data_{};
    std::string render_offset_{};

private:
    inline char abs( char const c ) {
        return c > 0 ? c : -c;
    }
    std::optional< char > get_desc( iterator const& it ) const
    {
        return it.desc_idx < rle_desc_.size()
            ? std::make_optional( rle_desc_[ it.desc_idx ] ) : std::nullopt;
    }
    std::optional< char > get_data( iterator const& it ) const
    {
        if ( auto const desc = get_desc( it ) )
        {
            auto const run = *desc > 0;
            return rle_data_[ it.data_idx + ( run ? 0 : it.counter ) ];
        }
        return {};
    }
    bool advance_right( iterator& it )
    {
        auto const desc = get_desc( it );
        if ( !desc ) {
            return false;
        }

        auto const run = *desc > 0;
        auto const count = abs( *desc );
        if ( ++it.counter >= count )
        {
            ++it.desc_idx;
            it.data_idx += ( run ? 1 : count );
            it.counter = 0;
        }
        return true;
    }
    bool advance_left( iterator& it )
    {
        if ( it.desc_idx == 0 && it.counter <= 0 ) {
            return false;
        }

        if ( it.counter > 0 ) {
            --it.counter;
        }
        else
        {
            --it.desc_idx;
            auto const desc = rle_desc_[ it.desc_idx ];
            it.data_idx += ( desc > 0 ? -1 : desc );
            // move desc cursor back and set count
            it.counter = abs( rle_desc_[ it.desc_idx ] ) - 1;
        }
        return true;
    }
    bool push_render_left()
    {
        if ( auto const v = get_data( begin_ ) ) {
            render_data_.push_front( *v );
            render_offset_.pop_back();
            return true;
        }
        return false;
    }
    bool push_render_right()
    {
        if ( auto const v = get_data( end_ ) ) {
            render_data_.push_back( *v );
            render_offset_.push_back( ' ' );
            return true;
        }
        return false;
    }
};


int main()
{
    {   // print raw data
        std::cout << "Encoded: ";
        for ( auto const& c : g_data ) {
            std::cout << static_cast< int >( c );
        }
        std::cout << std::endl;
    }

    {   // print decoded data
        std::cout << "Decoded: ";
        auto const rowinst = render_row{ g_data, 9999 };
        rowinst.render();
        std::cout << "\n";
    }

    {   // walk forward and backward
        std::cout << "2 fwd, 2 back (window 5):" << std::endl;
        auto rowinst = render_row{ g_data, 5 };
        rowinst.render();
        rowinst.advance_right();
        rowinst.render();
        rowinst.advance_right();
        rowinst.render();
        rowinst.advance_left();
        rowinst.render();
        rowinst.advance_left();
        rowinst.render();
        std::cout << std::endl;
    }

    {   // walk forward and backward
        std::cout << "To end, back to begin (window 8):" << std::endl;
        auto rowinst = render_row{ g_data, 8 };
        rowinst.render();
        while ( rowinst.advance_right() ) {
            rowinst.render();
        }
        while ( rowinst.advance_left() ) {
            rowinst.render();
        }
        std::cout << std::endl;
    }
}
