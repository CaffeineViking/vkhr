#include <vkhr/image.hh>

#define STBI_FAILURE_USERMSG
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include <stb_image.h>

#include <ctime>
#include <cstring>
#include <cstdio>

namespace vkhr {
    Image::Image(const unsigned width, const unsigned height)
                : width { width }, height { height } {
        image_data = new unsigned char[get_size_in_bytes()];
        is_stb_image = false; // We handle memory ourselves.
    }

    Image::Image(const std::string& file_path) {
        load(file_path); // iSTB handles memory.
    }

    Image::~Image() {
        free_image_buffers();
    }

    Image::Image(const Image& image) {
        width  = image.width;
        height = image.height;
        save_jpg_quality = image.save_jpg_quality;
        image_data = new unsigned char[image.get_size_in_bytes()];
        is_stb_image = false; // We do memory handling ourselves.
        std::memcpy(image_data, image.image_data,
                    image.get_size_in_bytes());
    }

    Image::Image(Image&& image) noexcept : Image {} {
        swap(*this, image);
    }

    Image& Image::operator=(Image image) {
        swap(*this, image);
        return *this;
    }

    void swap(Image& lhs, Image& rhs) {
        using std::swap;
        swap(lhs.width, rhs.width);
        swap(lhs.height, rhs.height);
        swap(lhs.is_stb_image, rhs.is_stb_image);
        swap(lhs.save_jpg_quality, rhs.save_jpg_quality);
        swap(lhs.image_data, rhs.image_data);
    }

    Image::operator bool() {
        return image_data;
    }

    const char* Image::get_failure_reason() const {
        return stbi_failure_reason();
    }

    bool Image::load(const std::string& file_path) {
        int width, height, channels_per_pixel;

        free_image_buffers(); // if there's any existing data here
        image_data = stbi_load(file_path.c_str(), &width, &height,
                               &channels_per_pixel, Channels);

        if (image_data == nullptr)
            return false;

        this->width  = static_cast<unsigned>(width);
        this->height = static_cast<unsigned>(height);
        is_stb_image = true;

        return true;
    }

    bool Image::save(const std::string& file_path) const {
        auto file_ext_pos = file_path.find_last_of('.');
        if (file_ext_pos == std::string::npos)
            return false;

        const auto extension = file_path.substr(file_ext_pos + 1);
        int error;

        if (extension == "png") error = stbi_write_png(file_path.c_str(), width, height,
                                                       Channels, image_data, 0);
        if (extension == "bmp") error = stbi_write_bmp(file_path.c_str(), width, height,
                                                       Channels, image_data);
        if (extension == "tga") error = stbi_write_tga(file_path.c_str(), width, height,
                                                       Channels, image_data);
        if (extension == "jpg") error = stbi_write_jpg(file_path.c_str(), width, height,
                                                       Channels, image_data,
                                                       save_jpg_quality);
        else return false; // Specify file extension.

        return !error;
    }

    bool Image::save_time() const {
        time_t current_time { time(0) };
        struct tm time_structure;
        char current_time_buffer[80];

        time_structure = *localtime(&current_time);
        strftime(current_time_buffer, sizeof(current_time_buffer),
                 "%F %H-%M-%S", &time_structure);

        std::string date { current_time_buffer };
        std::string file { date + ".png" };

        return save(file);
    }

    // Values between 0 -> 100 as the RFC.
    void Image::set_quality(int quality) {
        save_jpg_quality = quality;
    }

    unsigned Image::get_width() const {
        return width;
    }

    unsigned Image::get_pixel_count() const {
        return width * height;
    }

    float Image::get_aspect_ratio() const {
        return width / static_cast<float>(height);
    }

    unsigned Image::get_height() const {
        return height;
    }

    int Image::get_size_in_bytes() const {
        return get_pixel_count() * BytesPerPixel;
    }

    unsigned Image::get_expected_size(const unsigned width, const unsigned height) {
        return width * height * Image::BytesPerPixel;
    }

    unsigned char* Image::get_data() {
        return image_data;
    }

    const unsigned char* Image::get_data() const {
        return image_data;
    }

    Color* Image::get_pixels() {
        return reinterpret_cast<Color*>(image_data);
    }

    const Color* Image::get_pixels() const {
        return reinterpret_cast<Color*>(image_data);
    }

    const Color& Image::get_pixel(const int x,
                                  const int y) const {
        return get_pixels()[y * width + x];
    }

    void Image::set_pixel(const int x, const int y,
                          const Color& color) {
        get_pixels()[y * width + x] = color;
    }

    void Image::clear() {
        std::memset(image_data, ClearColor, get_size_in_bytes());
    }

    void Image::clear(const Color& color) {
        #pragma omp parallel for schedule(dynamic)
        for (int j = 0; j < get_height(); ++j)
        for (int i = 0; i < get_width();  ++i)
            set_pixel(i, j, color);
    }

    void Image::copy(const std::vector<glm::dvec3>& buffer, double samples) {
        #pragma omp parallel for schedule(dynamic)
        for (int j = 0; j < get_height(); ++j)
        for (int i = 0; i < get_width();  ++i) {
            std::size_t pixel { i + j * get_width() };
            set_pixel(get_width() - i - 1, j, {
                static_cast<unsigned char>(glm::clamp(buffer[pixel].r / samples, 0.0, 1.0) * 255.0),
                static_cast<unsigned char>(glm::clamp(buffer[pixel].g / samples, 0.0, 1.0) * 255.0),
                static_cast<unsigned char>(glm::clamp(buffer[pixel].b / samples, 0.0, 1.0) * 255.0),
                255
            });
        }
    }

    void Image::resize(const unsigned width, const unsigned height) {
        if (width == this->width && height == this->height)
            return; // We're done here folks!

        Image resized_image { width, height };

        const float x_ratio { width  / static_cast<float>(this->width) },
                    y_ratio { height / static_cast<float>(this->height) };

        #pragma omp parallel for schedule(dynamic)
        for (int j = 0; j < height; ++j)
        for (int i { 0 }; i < width;  ++i) {
            int nearest_i = static_cast<int>(i / y_ratio),
                nearest_j = static_cast<int>(j / x_ratio);
            auto nearest_pixel = get_pixel(nearest_i, nearest_j);
            resized_image.set_pixel(i, j, nearest_pixel);
        }

        *this = resized_image;
    }

    void Image::horizontal_flip() {
        unsigned half_width { width >> 1  };
        #pragma omp parallel for schedule(dynamic)
        for (int j = 0; j < height; ++j)
        for (int i { 0 }; i < half_width; ++i) {
            auto left_color  = get_pixel(i, j);
            auto right_color = get_pixel(width - i - 1, j);
            set_pixel(width - i - 1, j, left_color);
            set_pixel(i, j, right_color);
        }
    }

    void Image::vertical_flip() {
        // TODO: do this later.
    }

    void Image::flip_channels() {
        #pragma omp parallel for schedule(dynamic)
        for (int j = 0; j < height; ++j)
        for (int i { 0 }; i < width; ++i) {
            auto color = get_pixel(i, j);
            color.a = 255; // Alpha hack.
            std::swap(color.r, color.b);
            set_pixel(i, j, color);
        }
    }

    void Image::free_image_buffers() {
        if (image_data != nullptr) {
            if (!is_stb_image) {
                delete[] image_data;
            } else if (is_stb_image) {
                stbi_image_free(image_data);
            }
        }

        image_data = nullptr;
    }
}
