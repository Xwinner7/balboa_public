#include "hw1.h"
#include "hw1_scenes.h"

using namespace hw1;

Image3 hw_1_1(const std::vector<std::string> &params) {
    // Homework 1.1: render a circle at the specified
    // position, with the specified radius and color.

    Image3 img(640 /* width */, 480 /* height */);

    Vector2 center = Vector2{img.width / 2 + Real(0.5), img.height / 2 + Real(0.5)};
    Real radius = 100.0;
    Vector3 color = Vector3{1.0, 0.5, 0.5};
    for (int i = 0; i < (int)params.size(); i++) {
        if (params[i] == "-center") {
            Real x = std::stof(params[++i]);
            Real y = std::stof(params[++i]);
            center = Vector2{x, y};
        } else if (params[i] == "-radius") {
            radius = std::stof(params[++i]);
        } else if (params[i] == "-color") {
            Real r = std::stof(params[++i]);
            Real g = std::stof(params[++i]);
            Real b = std::stof(params[++i]);
            color = Vector3{r, g, b};
        }
    }
    
    Vector3 background_color = Vector3{0.5, 0.5, 0.5}; //Gray background

    //Render each pixel
    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            Real pixel_center_x = x + Real(0.5);
            Real pixel_center_y = (img.height - y - 1) + Real(0.5);
            Real dx = pixel_center_x - center.x;
            Real dy = pixel_center_y - center.y;
            Real distance_squared = dx * dx + dy * dy;
            if(distance_squared <= radius * radius) {
                img(x, y) = color;
            } else {
                img(x, y) = background_color;
            }
        }
    }
    return img;
}

Image3 hw_1_2(const std::vector<std::string> &params) {
    // Homework 1.2: render polylines
    if (params.size() == 0) {
        return Image3(0, 0);
    }

    Image3 img(640 /* width */, 480 /* height */);
    std::vector<Vector2> polyline;
    // is_closed = true indicates that the last point and
    // the first point of the polyline are connected
    bool is_closed = false;
    std::optional<Vector3> fill_color;
    std::optional<Vector3> stroke_color;
    Real stroke_width = 1;
    for (int i = 0; i < (int)params.size(); i++) {
        if (params[i] == "-points") {
            while (params.size() > i+1 &&
                    params[i+1].length() > 0 &&
                    params[i+1][0] != '-') {
                Real x = std::stof(params[++i]);
                Real y = std::stof(params[++i]);
                polyline.push_back(Vector2{x, y});
            }
        } else if (params[i] == "--closed") {
            is_closed = true;
        } else if (params[i] == "-fill_color") {
            Real r = std::stof(params[++i]);
            Real g = std::stof(params[++i]);
            Real b = std::stof(params[++i]);
            fill_color = Vector3{r, g, b};
        } else if (params[i] == "-stroke_color") {
            Real r = std::stof(params[++i]);
            Real g = std::stof(params[++i]);
            Real b = std::stof(params[++i]);
            stroke_color = Vector3{r, g, b};
        } else if (params[i] == "-stroke_width") {
            stroke_width = std::stof(params[++i]);
        }
    }
    // silence warnings, feel free to remove it
    UNUSED(stroke_width);

    if (fill_color && !is_closed) {
        std::cout << "Error: can't have a non-closed shape with fill color." << std::endl;
        return Image3(0, 0);
    }

    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            img(x, y) = Vector3{1, 1, 1};
        }
    }

    //Helper function
    auto point_in_polygon_nonzero = [](const Vector2& p, const std::vector<Vector2>& poly) {
        int winding = 0;
        int n = poly.size();
        for(int i = 0; i < n; i++) {
            Vector2 v1 = poly[i];
            Vector2 v2 = poly[(i+1)%n];
            if(v1.y <= p.y) {
                if(v2.y > p.y) {
                Real cross_product = (v2.x - v1.x) * (p.y - v1.y) - (v2.y - v1.y) * (p.x - v1.x);
                    if(cross_product > 0) {
                    winding++;
                    }
                }
            } else {
                if(v2.y <= p.y) {
                Real cross_product = (v2.x - v1.x) * (p.y - v1.y) - (v2.y - v1.y) * (p.x - v1.x);
                if(cross_product < 0) {
                    winding--;
                    }
                }
            } 
        }
        return winding != 0;
    };

    auto point_to_segment_distance = [](const Vector2& p, const Vector2& a, const Vector2& b) {
        Vector2 ab = b - a;
        Real len2 = dot(ab, ab);
        if(len2 == 0) return length(p - a);
        Real t = std::clamp(dot(p - a, ab) / len2, Real(0), Real(1));
        return length(p - (a + t * ab));
    };

    //Fill
    if(fill_color && is_closed && polyline.size() >= 3) {
        for(int y = 0; y < img.height; y++) {
            for(int x = 0; x < img.width; x++) {
                Vector2 p{x + 0.5, (img.height - y - 1) + 0.5};
                if(point_in_polygon_nonzero(p, polyline)) {
                    img(x, y) = *fill_color;
                }
            }
        }
    }

    //Stroke
    if(stroke_color && polyline.size() >= 2) {
        Real half_w = stroke_width / 2;
        size_t n = polyline.size();
        for(int y = 0; y < img.height; y++) {
            for(int x = 0; x < img.width; x++) {
                Vector2 p{x + 0.5, (img.height - y - 1) + 0.5};
                for(size_t i = 0; i < n - 1; i++) {
                    if(point_to_segment_distance(p, polyline[i], polyline[i + 1]) <= half_w) {
                        img(x, y) = *stroke_color;
                        break;
                    }
                }
                if(is_closed && n >= 3) {
                    if(point_to_segment_distance(p, polyline.back(), polyline.front()) <= half_w) {
                        img(x, y) = *stroke_color;
                    }
                }
                for(size_t i = 0; i < n; i++) {
                    if(length(p - polyline[i]) <= half_w) {
                        img(x, y) = *stroke_color;
                        break;
                    }
                }
            }
        }
    }
    return img;
}

Image3 hw_1_3(const std::vector<std::string> &params) {
    // Homework 1.3: render multiple shapes
    if (params.size() == 0) {
        return Image3(0, 0);
    }

    Scene scene = parse_scene(params[0]);
    std::cout << scene << std::endl;

    Image3 img(scene.resolution.x, scene.resolution.y);

    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            img(x, y) = scene.background;
        }
    }
    //Helper function
    auto point_in_polygon_nonzero = [](const Vector2& p, const std::vector<Vector2>& poly) {
        int winding = 0;
        int n = poly.size();
        for (int i = 0; i < n; i++) {
            Vector2 v1 = poly[i];
            Vector2 v2 = poly[(i + 1) % n];
            if (v1.y <= p.y) {
                if (v2.y > p.y) {
                    Real cross_product = (v2.x - v1.x) * (p.y - v1.y) - (v2.y - v1.y) * (p.x - v1.x);
                    if (cross_product > 0) winding++;
                }
            } else {
                if (v2.y <= p.y) {
                    Real cross_product = (v2.x - v1.x) * (p.y - v1.y) - (v2.y - v1.y) * (p.x - v1.x);
                    if (cross_product < 0) winding--;
                }
            }
        }
        return winding != 0;
    };

    auto point_to_segment_distance = [](const Vector2& p, const Vector2& a, const Vector2& b) {
        Vector2 ab = b - a;
        Real len2 = dot(ab, ab);
        if (len2 == 0) return length(p - a);
        Real t = std::clamp(dot(p - a, ab) / len2, Real(0), Real(1));
        return length(p - (a + t * ab));
    };

    //Render shapes
    for(int i = scene.shapes.size() - 1; i >= 0; i--) {
        const Shape& shape = scene.shapes[i];
        if(auto* circle = std::get_if<Circle>(&shape)) {
            if(circle->fill_color) {
                for(int y = 0; y < img.height; y++) {
                    for(int x = 0; x < img.width; x++) {
                        Vector2 p{x + 0.5, (img.height - y - 1) + 0.5};
                        Real dx = p.x - circle->center.x;
                        Real dy = p.y - circle->center.y;
                        if(dx * dx + dy * dy <= circle->radius * circle->radius) {
                            img(x, y) = *circle->fill_color;
                        }
                    }
                }
            }
            if(circle->stroke_color) {
                Real half_w = circle->stroke_width / 2;
                for(int y = 0; y < img.height; y++) {
                    for(int x = 0; x < img.width; x++) {
                        Vector2 p{x + 0.5, (img.height - y - 1) + 0.5};
                        Real dx = p.x - circle->center.x;
                        Real dy = p.y - circle->center.y;
                        Real dist = std::sqrt(dx * dx + dy * dy);
                        if(std::abs(dist - circle->radius) <= half_w) {
                            img(x, y) = *circle->stroke_color;
                        }
                    }
                }
            }
        } else if(auto* polyline = std::get_if<Polyline>(&shape)) {
            if(polyline->fill_color && polyline->is_closed && polyline->points.size() >= 3) {
                for(int y = 0; y < img.height; y++) {
                    for(int x = 0; x < img.width; x++) {
                        Vector2 p{x + 0.5, (img.height - y - 1) + 0.5};
                        if(point_in_polygon_nonzero(p, polyline->points)) {
                            img(x, y) = *polyline->fill_color;
                        }
                    }
                }
            }
            if(polyline->stroke_color && polyline->points.size() >= 2) {
                Real half_w = polyline->stroke_width / 2;
                size_t n = polyline->points.size();
                for(int y = 0; y < img.height; y++) {
                    for(int x = 0; x < img.width; x++) {
                        Vector2 p{x + 0.5, (img.height - y - 1) + 0.5};
                        for(size_t i = 0; i < n - 1; i++) {
                            if(point_to_segment_distance(p, polyline->points[i], polyline->points[i + 1]) <= half_w){
                                img(x, y) = *polyline->stroke_color;
                                break;
                            }
                        }
                        if(polyline->is_closed && n >= 3) {
                            if(point_to_segment_distance(p, polyline->points.back(), polyline->points.front()) <= half_w) {
                                img(x, y) = *polyline->stroke_color;
                                continue;
                            }
                        }
                        for(size_t i = 0; i < n; i++) {
                            if(length(p - polyline->points[i]) <= half_w) {
                                img(x, y) = *polyline->stroke_color;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    return img;
}

Image3 hw_1_4(const std::vector<std::string> &params) {
    // Homework 1.4: render transformed shapes
    if (params.size() == 0) {
        return Image3(0, 0);
    }

    Scene scene = parse_scene(params[0]);
    std::cout << scene << std::endl;

    Image3 img(scene.resolution.x, scene.resolution.y);

    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            img(x, y) = scene.background;
        }
    }

    //Helper functions
    auto point_in_polygon_nonzero = [](const Vector2& p, const std::vector<Vector2>& poly) {
        int winding = 0;
        int n = poly.size();
        for (int i = 0; i < n; i++) {
            Vector2 v1 = poly[i];
            Vector2 v2 = poly[(i + 1) % n];
            if (v1.y <= p.y) {
                if (v2.y > p.y) {
                    Real cross_product = (v2.x - v1.x) * (p.y - v1.y) - (v2.y - v1.y) * (p.x - v1.x);
                    if (cross_product > 0) winding++;
                }
            } else {
                if (v2.y <= p.y) {
                    Real cross_product = (v2.x - v1.x) * (p.y - v1.y) - (v2.y - v1.y) * (p.x - v1.x);
                    if (cross_product < 0) winding--;
                }
            }
        }
        return winding != 0;
    };

    auto point_to_segment_distance = [](const Vector2& p, const Vector2& a, const Vector2& b) {
        Vector2 ab = b - a;
        Real len2 = dot(ab, ab);
        if (len2 == 0) return length(p - a);
        Real t = std::clamp(dot(p - a, ab) / len2, Real(0), Real(1));
        return length(p - (a + t * ab));
    };
    
    auto point_in_line_segment_flat = [](const Vector2& p, const Vector2& a, const Vector2& b, Real half_width) -> bool {
        Vector2 ab = b - a;
        Real length_ab = length(ab);
        if (length_ab == 0) return false;
        Vector2 ab_normalized = ab / length_ab;
        Vector2 perpendicular = Vector2{-ab_normalized.y, ab_normalized.x};
        Vector2 ap = p - a;
        Real projection = dot(ap, ab_normalized);
        if (projection < 0 || projection > length_ab) {
            return false;
        }
        Real side_distance = std::abs(dot(ap, perpendicular));
        return side_distance <= half_width;
    };


    auto transform_point = [](const Matrix3x3& transform, const Vector2& point) -> Vector2 {
        Vector3 homogeneous{point.x, point.y, 1.0};
        Vector3 transformed = transform * homogeneous;
        return Vector2{transformed.x / transformed.z, transformed.y / transformed.z};
    };

    //Render each shape
    for(auto it = scene.shapes.rbegin(); it != scene.shapes.rend(); ++it) {
        const Shape& shape = *it;
        if(auto* circle = std::get_if<Circle>(&shape)) {
            Matrix3x3 inverse_transform = inverse(circle->transform);
            if(circle->fill_color) {
                for(int y = 0; y < img.height; y++) {
                    for(int x = 0; x < img.width; x++) {
                        Vector2 canvas_point{x + 0.5, (img.height - y - 1) + 0.5};
                        Vector2 object_point = transform_point(inverse_transform, canvas_point);
                        Real dx = object_point.x - circle->center.x;
                        Real dy = object_point.y - circle->center.y;
                        if(dx * dx + dy * dy <= circle->radius * circle->radius) {
                            img(x, y) = *circle->fill_color;
                        }
                    }
                }
            }
            if(circle->stroke_color) {
                Real half_w = circle->stroke_width / 2;
                for(int y = 0; y < img.height; y++) {
                    for(int x = 0; x < img.width; x++) {
                        Vector2 canvas_point{x + 0.5, (img.height - y - 1) + 0.5};
                        Vector2 object_point = transform_point(inverse_transform, canvas_point);
                        Real dx = object_point.x - circle->center.x;
                        Real dy = object_point.y - circle->center.y;
                        Real dist = std::sqrt(dx * dx + dy * dy);
                        if(std::abs(dist - circle->radius) <= half_w) {
                            img(x, y) = *circle->stroke_color;
                        }
                    }
                }
            }
        } else if(auto* polyline = std::get_if<Polyline>(&shape)) {
            Matrix3x3 inverse_transform = inverse(polyline->transform);
            if(polyline->fill_color && polyline->is_closed && polyline->points.size() >= 3) {
                for(int y = 0; y < img.height; y++) {
                    for(int x = 0; x < img.width; x++) {
                        Vector2 canvas_point{x + 0.5, (img.height - y - 1) + 0.5};
                        Vector2 object_point = transform_point(inverse_transform, canvas_point);
                        if(point_in_polygon_nonzero(object_point, polyline->points)) {
                            img(x, y) = *polyline->fill_color;
                        }
                    }
                }
            }
            if(polyline->stroke_color && polyline->points.size() >= 2) {
                Real half_w = polyline->stroke_width / 2.0;
                size_t n = polyline->points.size();
                for(int y = 0; y < img.height; y++) {
                    for(int x = 0; x < img.width; x++) {
                        Vector2 canvas_point{x + 0.5, (img.height - y - 1) + 0.5};
                        Vector2 object_point = transform_point(inverse_transform, canvas_point);
                        for(size_t i = 0; i < n - 1; i++) {
                            if(point_in_line_segment_flat(object_point, polyline->points[i], polyline->points[i + 1], half_w)) {
                                img(x, y) = *polyline->stroke_color;
                                break;
                            }
                        }
                        if(polyline->is_closed && n >= 3) {
                            if(point_in_line_segment_flat(object_point, polyline->points.back(), polyline->points.front(), half_w)) {
                                img(x, y) = *polyline->stroke_color;
                            }
                        }
                    }
                }
            }
        }
    }
    return img;
}

Image3 hw_1_5(const std::vector<std::string> &params) {
    // Homework 1.5: antialiasing
    if (params.size() == 0) {
        return Image3(0, 0);
    }

    Scene scene = parse_scene(params[0]);
    std::cout << scene << std::endl;

    Image3 img(scene.resolution.x, scene.resolution.y);

    //Helper functions
    auto point_in_polygon_nonzero = [](const Vector2& p, const std::vector<Vector2>& poly) {
        int winding = 0;
        int n = poly.size();
        for (int i = 0; i < n; i++) {
            Vector2 v1 = poly[i];
            Vector2 v2 = poly[(i + 1) % n];
            if (v1.y <= p.y) {
                if (v2.y > p.y) {
                    Real cross_product = (v2.x - v1.x) * (p.y - v1.y) - (v2.y - v1.y) * (p.x - v1.x);
                    if (cross_product > 0) winding++;
                }
            } else {
                if (v2.y <= p.y) {
                    Real cross_product = (v2.x - v1.x) * (p.y - v1.y) - (v2.y - v1.y) * (p.x - v1.x);
                    if (cross_product < 0) winding--;
                }
            }
        }
        return winding != 0;
    };

    auto point_in_line_segment_flat = [](const Vector2& p, const Vector2& a, const Vector2& b, Real half_width) -> bool {
        Vector2 ab = b - a;
        Real length_ab = length(ab);
        if (length_ab == 0) return false;
        
        Vector2 ab_normalized = ab / length_ab;
        Vector2 perpendicular = Vector2{-ab_normalized.y, ab_normalized.x};
        
        Vector2 ap = p - a;
        
        Real projection = dot(ap, ab_normalized);
        if (projection < 0 || projection > length_ab) {
            return false;
        }
        
        Real side_distance = std::abs(dot(ap, perpendicular));
        return side_distance <= half_width;
    };

    auto transform_point = [](const Matrix3x3& transform, const Vector2& point) -> Vector2 {
        Vector3 homogeneous{point.x, point.y, 1.0};
        Vector3 transformed = transform * homogeneous;
        return Vector2{transformed.x / transformed.z, transformed.y / transformed.z};
    };

    auto compute_color_at_point = [&](const Vector2& canvas_point) -> Vector3 {Vector3 color = scene.background;
        for(auto it = scene.shapes.rbegin(); it != scene.shapes.rend(); ++it) {
            const Shape& shape = *it;
            if(auto* circle = std::get_if<Circle>(&shape)) {
                Matrix3x3 inverse_transform = inverse(circle->transform);
                Vector2 object_point = transform_point(inverse_transform, canvas_point);
                Real dx = object_point.x - circle->center.x;
                Real dy = object_point.y - circle->center.y;
                Real dist_sq = dx * dx + dy * dy;
                if(circle->fill_color && dist_sq <= circle->radius * circle->radius) {
                    color = *circle->fill_color;
                }
                if(circle->stroke_color) {
                    Real dist = std::sqrt(dist_sq);
                    Real half_w = circle->stroke_width / 2;
                    if(std::abs(dist - circle->radius) <= half_w) {
                        color = *circle->stroke_color;
                    }
                }
            } else if(auto* polyline = std::get_if<Polyline>(&shape)) {
                Matrix3x3 inverse_transform = inverse(polyline->transform);
                Vector2 object_point = transform_point(inverse_transform, canvas_point);
                if(polyline->fill_color && polyline->is_closed && polyline->points.size() >= 3) {
                    if(point_in_polygon_nonzero(object_point, polyline->points)) {
                        color = *polyline->fill_color;
                    }
                }
                if(polyline->stroke_color && polyline->points.size() >= 2) {
                    Real half_w = polyline->stroke_width / 2.0;
                    size_t n = polyline->points.size();
                    for(size_t i = 0; i < n - 1; i++) {
                        if(point_in_line_segment_flat(object_point, polyline->points[i], polyline->points[i+1], half_w)) {
                            color = *polyline->stroke_color;
                            break;
                        }
                    }
                    if(polyline->is_closed && n >= 3) {
                        if(point_in_line_segment_flat(object_point, polyline->points.back(), polyline->points.front(), half_w)) {
                            color = *polyline->stroke_color;
                        }
                    }
                }
            }
        }
        return color;
    };

    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            Vector3 accumulated = Vector3{0, 0, 0};
            for(int sy = 0; sy < 4; sy++) {
                for(int sx = 0; sx < 4; sx++) {
                    Vector2 p{
                        x + (sx + 0.5) / 4.0,
                        (img.height - y - 1) + (sy + 0.5) / 4.0
                    };
                    accumulated+= compute_color_at_point(p);
                }
            }
            img(x, y) = accumulated / 16.0;
        }
    }
    return img;
}

Image3 hw_1_6(const std::vector<std::string> &params) {
    // Homework 1.6: alpha blending
    if (params.size() == 0) {
        return Image3(0, 0);
    }

    Scene scene = parse_scene(params[0]);
    std::cout << scene << std::endl;

    Image3 img(scene.resolution.x, scene.resolution.y);

    //Helper functions
    auto point_in_polygon_nonzero = [](const Vector2& p, const std::vector<Vector2>& poly) {
        int winding = 0;
        int n = poly.size();
        for(int i = 0; i < n; i++) {
            Vector2 v1 = poly[i];
            Vector2 v2 = poly[(i + 1) % n];
            if(v1.y <= p.y) {
                if(v2.y > p.y) {
                    Real cross_product = (v2.x - v1.x) * (p.y - v1.y) - (v2.y - v1.y) * (p.x - v1.x);
                    if(cross_product > 0) winding++;
                }
            } else {
                if(v2.y <= p.y) {
                    Real cross_product = (v2.x - v1.x) * (p.y - v1.y) - (v2.y - v1.y) * (p.x - v1.x);
                    if(cross_product < 0) winding--;
                }
            }
        }
        return winding != 0;
    };

    auto point_in_line_segment_flat = [](const Vector2& p, const Vector2& a, const Vector2& b, Real half_width) -> bool {
        Vector2 ab = b - a;
        Real length_ab = length(ab);
        if(length_ab == 0) return false;
        Vector2 ab_normalized = ab / length_ab;
        Vector2 perpendicular = Vector2{-ab_normalized.y, ab_normalized.x};
        Vector2 ap = p - a;
        Real projection = dot(ap, ab_normalized);
        if(projection < 0 || projection > length_ab) {
            return false;
        }
        Real side_distance = std::abs(dot(ap, perpendicular));
        return side_distance <= half_width;
    };

    auto transform_point = [](const Matrix3x3& transform, const Vector2& point) -> Vector2 {
        Vector3 homogeneous{point.x, point.y, 1.0};
        Vector3 transformed = transform * homogeneous;
        return Vector2{transformed.x / transformed.z, transformed.y / transformed.z};
    };

    // Function to compute color with alpha blending at a specific canvas point
    auto compute_color_at_point = [&](const Vector2& canvas_point) -> Vector3 {
        Vector3 color = scene.background;
        Real accumulated_alpha = 1.0;
        
        for(auto it = scene.shapes.rbegin(); it != scene.shapes.rend(); ++it) {
            const Shape& shape = *it;
            
            if(auto* circle = std::get_if<Circle>(&shape)) {
                Matrix3x3 inverse_transform = inverse(circle->transform);
                Vector2 object_point = transform_point(inverse_transform, canvas_point);
                Real dx = object_point.x - circle->center.x;
                Real dy = object_point.y - circle->center.y;
                Real dist_sq = dx * dx + dy * dy;
                if(circle->fill_color && dist_sq <= circle->radius * circle->radius) {
                    Real fill_alpha = std::clamp(circle->fill_alpha, Real(0), Real(1));
                    color = fill_alpha * (*circle->fill_color) + (1 - fill_alpha) * color;
                    accumulated_alpha *= (1 - fill_alpha);
                }
                if(circle->stroke_color) {
                    Real dist = std::sqrt(dist_sq);
                    Real half_w = circle->stroke_width / 2;
                    if(std::abs(dist - circle->radius) <= half_w) {
                        Real stroke_alpha = std::clamp(circle->stroke_alpha, Real(0), Real(1));
                        color = stroke_alpha * (*circle->stroke_color) + (1 - stroke_alpha) * color;
                        accumulated_alpha *= (1 - stroke_alpha);
                    }
                }
            } else if(auto* polyline = std::get_if<Polyline>(&shape)) {
                Matrix3x3 inverse_transform = inverse(polyline->transform);
                Vector2 object_point = transform_point(inverse_transform, canvas_point);
                if(polyline->fill_color && polyline->is_closed && polyline->points.size() >= 3) {
                    if(point_in_polygon_nonzero(object_point, polyline->points)) {
                        Real fill_alpha = std::clamp(polyline->fill_alpha, Real(0), Real(1));
                        color = fill_alpha * (*polyline->fill_color) + (1 - fill_alpha) * color;
                        accumulated_alpha *= (1 - fill_alpha);
                    }
                }
                if(polyline->stroke_color && polyline->points.size() >= 2) {
                    Real half_w = polyline->stroke_width / 2.0;
                    size_t n = polyline->points.size();
                    bool in_stroke = false;
                    for(size_t i = 0; i < n - 1; i++) {
                        if(point_in_line_segment_flat(object_point, polyline->points[i], polyline->points[i + 1], half_w)) {
                            in_stroke = true;
                            break;
                        }
                    }
                    if(!in_stroke && polyline->is_closed && n >= 3) {
                        if(point_in_line_segment_flat(object_point, polyline->points.back(), polyline->points.front(), half_w)) {
                            in_stroke = true;
                        }
                    }
                    if(in_stroke) {
                        Real stroke_alpha = std::clamp(polyline->stroke_alpha, Real(0), Real(1));
                        color = stroke_alpha * (*polyline->stroke_color) + (1 - stroke_alpha) * color;
                        accumulated_alpha *= (1 - stroke_alpha);
                    }
                }
            }
            if(accumulated_alpha <= 0.0) {
                break;
            }
        }
        return color;
    };

    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            Vector3 accumulated = Vector3{0, 0, 0};
            for(int sy = 0; sy < 4; sy++) {
                for(int sx = 0; sx < 4; sx++) {
                    Vector2 p{
                        x + (sx + 0.5) / 4.0, (img.height - y - 1) + (sy + 0.5) / 4.0
                    };
                    accumulated += compute_color_at_point(p);
                }
            }
            img(x, y) = accumulated / 16.0;
        }
    }
    return img;
}
