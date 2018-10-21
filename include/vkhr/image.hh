#ifndef VKHR_IMAGE_HH
#define VKHR_IMAGE_HH

#include <glm/glm.hpp>

#include <string>

namespace vkhr {
    using Color = glm::tvec4<unsigned char>;

    class Image final {
    public:
        Image() = default;
        Image(const int width,
              const int height);
        Image(const std::string& file_path);
        ~Image(); // deletes/stb_image_free.

        Image(const Image& image);
        Image(Image&& image) noexcept;
        Image& operator=(Image image);

        friend void swap(Image& lhs, Image& rhs);

        operator bool(); // if load went OK.

        const char* get_failure_reason() const;

        static constexpr int ClearColor { 0x00 };
        static constexpr int Channels { 4 };
        static constexpr int BytesPerChannel { 1 };
        static constexpr int BytesPerPixel {
            Channels * BytesPerChannel
        };

        bool load(const std::string& file_path);
        bool save(const std::string& file_path) const;

        void set_quality(int quality); // saving JPEG.

        int get_width() const;
        int get_pixel_count() const;
        int get_height() const;

        float get_aspect_ratio() const;

        unsigned char* get_data();
        const unsigned char* get_data() const;
        int get_size_in_bytes() const;

        Color* get_pixels();
        const Color* get_pixels() const;
        const Color& get_pixel(const int x,
                               const int y) const;
        void set_pixel(const int x, const int y,
                       const Color& color);

        void clear();
        void clear(const Color& color);

        // TODO: support bilinear and bicubic interpolation later.
        void resize(const int width, const int height);

        template<typename F> void filter_neighborhood(F functor);
        template<typename F> void filter(F functor);

    private:
        void free_image_buffers();
        int width { 0 }, height { 0 };
        bool is_stb_image { false };
        int save_jpg_quality { 90 };
        unsigned char* image_data { nullptr };
    };

    template<typename F>
    void Image::filter_neighborhood(F functor) {
        for (int j = 0; j < get_height(); ++j)
        for (int i = 0; i < get_width();  ++i) {
            Color neighborhood_pixels[9];
            constexpr int s = 3;

            for (int y = -1; y <= 1; ++y)
            for (int x = -1; x <= 1; ++x) {
                const int p { (y + 1)*s + (x + 1) };
                if ((i + x) < 0 || (i + x) > get_width() ||
                    (j + y) < 0 || (j + y) > get_height()) {
                    neighborhood_pixels[p] = { 0, 0, 0, 1 };
                } else {
                    neighborhood_pixels[p] = get_pixel(i+x, j+y);
                }
            }

            set_pixel(i, j, functor(i, j, get_pixel(i, j),
                                    neighborhood_pixels));
        }
    }

    template<typename F>
    void Image::filter(F functor) {
        for (int j { 0 }; j < get_height(); ++j)
        for (int i { 0 }; i < get_width();  ++i) {
            set_pixel(i, j,  functor(i, j, get_pixel(i, j)));
        }
    }
}

#endif
