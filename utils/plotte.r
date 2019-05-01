library(plyr)
library(dplyr)
library(reshape2)
library(RColorBrewer)
library(ggplot2)
library(forcats)
library(scales)

benchmark_files <- file.info(list.files(".", pattern="\\.csv$", full.names=T))
benchmark_file  <- rownames(benchmark_files)[which.max(benchmark_files$mtime)]

benchmark <- read.csv(benchmark_file, strip.white=TRUE)

memory_breakdown <- benchmark %>%
                    filter(Description=="Time (ms)", Renderer=="Rasterizer") %>%
                    select(Scene, PPLL, Geometry, Volume)
time_vs_strands  <- benchmark %>%
                    filter(Description=="Time (ms) vs. Strands") %>%
                    select(Renderer, Scene,  Strands, Pixels,  Total.Frame.Time,
                           Bake.Shadow.Maps, Voxelize.Strands, Clear.PPLL.Nodes,
                           Draw.Hair.Styles, Raymarch.Strands, Resolve.the.PPLL)
time_vs_distance <- benchmark %>%
                    filter(Description=="Time (ms) vs. Distance") %>%
                    select(Renderer, Scene,  Distance, Pixels, Total.Frame.Time,
                           Bake.Shadow.Maps, Voxelize.Strands, Clear.PPLL.Nodes,
                           Draw.Hair.Styles, Raymarch.Strands, Resolve.the.PPLL)
time_vs_samples  <- benchmark %>%
                    filter(Description=="Time (ms) vs. Samples") %>%
                    select(Renderer, Scene,  Samples,  Pixels, Total.Frame.Time,
                           Bake.Shadow.Maps, Voxelize.Strands, Clear.PPLL.Nodes,
                           Draw.Hair.Styles, Raymarch.Strands, Resolve.the.PPLL)
render_breakdown <- benchmark %>%
                    filter(Description=="Time (ms)") %>%
                    select(Renderer, Scene,  Bake.Shadow.Maps, Voxelize.Strands,
                           Clear.PPLL.Nodes, Draw.Hair.Styles, Raymarch.Strands,
                           Resolve.the.PPLL)

memory_breakdown <- melt(memory_breakdown, id=c("Scene"))
memory_breakdown <- ddply(memory_breakdown, .(Scene), transform, pos=value+(0.5*value))

ggplot(memory_breakdown, aes(x="", y=value, fill=variable)) +
    geom_bar(stat="identity", position=position_fill(reverse=TRUE)) +
    facet_grid(. ~ Scene) +
    theme(axis.title.x=element_blank(), axis.title.y=element_blank(), panel.border=element_blank(), panel.grid=element_blank(), axis.ticks=element_blank(), axis.text.x=element_blank()) +
    coord_polar(theta="y") +
    labs(fill="Device Data\n") +
    scale_fill_brewer(palette="Set2") +
    geom_text(data=memory_breakdown, col="gray16", aes(y=pos, label=sprintf("%.0f MB", value / 1000^2)), position=position_fill(vjust=0.5, reverse=TRUE))

minimum_ponytail_distance <- min((time_vs_distance %>% filter(Scene=="Ponytail"))$Distance)
screen_pixels <- first(benchmark$Width) * first(benchmark$Height)
maximum_ponytail_distance <- max((time_vs_distance %>% filter(Scene=="Ponytail"))$Distance)

render_breakdown <- time_vs_distance %>%
                    filter(Distance==minimum_ponytail_distance | Distance==maximum_ponytail_distance) %>%
                    select(Renderer, Distance, Pixels, Bake.Shadow.Maps,
                           Voxelize.Strands, Clear.PPLL.Nodes,
                           Draw.Hair.Styles, Raymarch.Strands,
                           Resolve.the.PPLL)

names(render_breakdown) <- gsub(x = names(render_breakdown), pattern = "\\.", replacement = " ")

ponytail_min_coverage <- percent(first(render_breakdown$Pixels[render_breakdown$Distance==minimum_ponytail_distance]) / screen_pixels)
ponytail_max_coverage <- percent(first(render_breakdown$Pixels[render_breakdown$Distance==maximum_ponytail_distance]) / screen_pixels)

render_breakdown$Distance[render_breakdown$Distance==minimum_ponytail_distance] <- paste("Near (Covers", ponytail_min_coverage, "of the Screen)")
render_breakdown$Distance[render_breakdown$Distance==maximum_ponytail_distance] <- paste("Far (Covers",  ponytail_max_coverage, "of the Screen)")

render_breakdown <- melt(render_breakdown, id.vars=c("Renderer", "Distance", "Pixels"))
render_breakdown <- ddply(render_breakdown, .(Distance, Renderer), transform, pos=cumsum(value) - (0.5*value))

ggplot(render_breakdown, aes(x=Renderer, y=value, fill=variable)) +
    geom_bar(stat="identity", position=position_stack(reverse=TRUE)) +
    facet_grid(. ~ Distance) +
    scale_fill_brewer(palette="Set3") +
    labs(x=NULL, y="Rendering Time (ms)\n", fill="Pass\n") +
    geom_text(data=render_breakdown, col="gray16", aes(x=Renderer, y=pos, label=ifelse(value > 0.5, sprintf("%.1fms", value), "")))

ggplot(time_vs_distance %>% filter(Scene=="Bear"), aes(x=Pixels,   y=Total.Frame.Time, col=Renderer, label=Total.Frame.Time)) +
    labs(x="\nPixels Covered", y="Rendering Time (ms)\n", col="Renderer Type\n") +
    scale_x_continuous(labels=comma) +
    geom_line(size=0.75)

ggplot(time_vs_distance %>% filter(Scene=="Ponytail"), aes(x=Distance, y=Total.Frame.Time, col=Renderer)) +
    labs(x="\nDistance (Scene Units)", y="Rendering Time (ms)\n", col="Renderer Type\n") +
    scale_x_continuous(labels=comma) +
    geom_line(size=0.75)

ggplot(time_vs_strands %>% filter(Scene=="Ponytail"), aes(x=Strands, y=Total.Frame.Time, col=Renderer)) +
    labs(x="\nHair Strands", y="Rendering Time (ms)\n", col="Renderer Type\n") +
    scale_x_continuous(labels=comma) +
    geom_line(size=0.75)

ggplot(time_vs_strands %>% filter(Scene=="Ponytail", Renderer=="Raymarcher"), aes(x=Strands, y=Voxelize.Strands)) +
    labs(x="\nHair Strands", y="Voxelization (ms)\n") +
    scale_x_continuous(labels=comma) +
    geom_line(size=0.75, col="#7CAE00")

ggplot(time_vs_samples %>% filter(Scene=="Ponytail", Renderer=="Raymarcher"), aes(x=Samples, y=Total.Frame.Time)) +
    labs(x="\nRaymarch Steps", y="Raymarch Time (ms)\n") +
    geom_line(size=0.75, col="#C77CFF")
