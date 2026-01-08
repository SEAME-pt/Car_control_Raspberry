# Assets Directory

This directory contains all the graphical assets used in the Qt HMI Display UI project.

## File Descriptions

### Primary Dashboard Graphics
- **Dashboard.svg**: Main dashboard background and layout
- **background.svg**: Speedometer gauge background
- **Vector 70.svg**: Top bar background element

### Vehicle Graphics
- **Car.svg**: Main vehicle silhouette
- **Model 3.png**: Tesla Model 3 vehicle image
- **Model 3.svg**: Vector version of Model 3
- **newcar.svg**: Alternative car graphic
- **car.png**: Raster car image

### Road and Path Elements
- **Vector 1.svg**: Right road lane graphic
- **Vector 2.svg**: Left road lane graphic

### Gauge Components
- **needle.svg**: Speedometer needle graphic
- **tickmark.svg**: Gauge tick marks
- **background.png**: Alternative background image

### Status Icons
- **Low beam headlights.svg**: Headlight indicator (active state)
- **Low_beam_headlights_white.svg**: Headlight indicator (inactive state)

### Statistics Icons
- **road.svg**: Distance/road icon
- **fuel.svg**: Fuel efficiency icon  
- **speedometer.svg**: Average speed icon

### Unused/Alternative Assets
- **FourthRightIcon.svg**: Additional UI element
- **thirdRightIcon.svg**: UI element (normal state)
- **thirdRightIcon_red.svg**: UI element (alert state)

## File Formats

### SVG Files
- Scalable vector graphics
- Preferred format for UI elements
- Crisp rendering at any size
- Small file sizes

### PNG Files
- Raster images for photographic content
- Used for complex illustrations
- Higher quality but larger file sizes

## Usage in QML

Assets are referenced using the Qt Resource System:

```qml
Image {
    source: "qrc:/assets/Dashboard.svg"
    asynchronous: true
}
```

## Design Guidelines

### Colors
- Primary: Dark theme (#1E1E1E)
- Accent: Cyan (#01E6DE)
- Success: Green (#32D74B)
- Warning: Yellow
- Error: Red
- Text: White (#FFFFFF)

### Style
- Modern automotive design
- Electric vehicle aesthetic
- Clean geometric shapes
- Minimal visual complexity
- High contrast for visibility

## Asset Optimization

### Best Practices
- Use SVG for icons and simple graphics
- Optimize SVG files to remove unnecessary elements
- Use appropriate PNG compression for raster images
- Consider file sizes for performance
- Maintain consistent visual style

### Performance Tips
- Set `asynchronous: true` for large images
- Use `sourceSize` to control rendering size
- Cache frequently used assets
- Minimize complex path operations in SVG

## Licensing

All assets in this directory are either:
- Created specifically for this project (MIT License)
- Sourced from public domain collections
- Licensed under compatible open source licenses

See the main LICENSE file for complete licensing information.

## Contributing Assets

When adding new assets:
1. Follow the established naming convention
2. Use appropriate file formats (SVG preferred)
3. Maintain visual consistency with existing assets
4. Optimize file sizes
5. Update this Info.txt file
6. Ensure proper licensing

For more information, see the project's CONTRIBUTING.md file.