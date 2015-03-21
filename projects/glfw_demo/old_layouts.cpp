double
Graph::create_cluster_ring(std::vector<Node*> current_cluster, double globalPositionX, double globalPositionY, unsigned i_ring)
{
  double ring_factor = 1;

  if (i_ring != 0)
    ring_factor = (i_ring+1) *100;

  double width_height = (100 * ring_factor) * sqrt(current_cluster.size());
  double angle = (M_PI * 2.0)/current_cluster.size();
  double radius = 100 * current_cluster.size() + width_height;

  for(unsigned node_i = 0; node_i != current_cluster.size(); node_i++)
  {
    Node* current_node = current_cluster[node_i];

    current_node->_x = (cos(angle * node_i) * radius) + globalPositionX;
    current_node->_y = (sin(angle * node_i) * radius) + globalPositionY;

  }

  return radius;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
// Ring Layout
/////////////////////////////////////////////////////////////////////////////////////////////////

void
Graph::make_ring_layout()
{
  double globalPositionX = 10.0;
  double globalPositionY = 10.0;

  double maxRadius = 0.0;

  for (unsigned i_cluster = 0; i_cluster != _all_clusters.size(); ++i_cluster)
  {
    std::vector<Node*> current_cluster = _all_clusters[i_cluster];

    double radius = 0.0;

    // Create multiple rings if cluster has more than 20 nodes
    if(current_cluster.size() > 20)
    {
      unsigned cluster_node_weights_sum = 0; // Number of edges of cluster
      double min_edge_weigth = 1.1; // Minimum edge weight of cluster
      double max_edge_weight = 0.0; // Maximum edge weight of cluster

      for(unsigned node_i = 0; node_i != current_cluster.size(); node_i++)
      {
        Node* current_node = current_cluster[node_i];

        cluster_node_weights_sum += current_node->getDegree();

        max_edge_weight = std::max(max_edge_weight, get_max_edge_weight(current_node));
        min_edge_weigth = std::min(min_edge_weigth, get_min_edge_weight(current_node));
      }

      // Fill inner ring and outer rings with nodes
      std::vector<Node*> inner_ring;
      std::vector<Node*> outer_rings;

      unsigned inner_ring_node_weights_sum = 0;
      for (unsigned node_i = 0; node_i != current_cluster.size(); node_i++)
      {
        Node* current_node = current_cluster[node_i];

        // if(node_i != 0) // Für 1 Node in der Mitte
        if(inner_ring_node_weights_sum >= cluster_node_weights_sum * 0.5)
          outer_rings.push_back(current_node);

        else
        {
          inner_ring.push_back(current_node);

          current_node->_color[0] = 1.0f;
          current_node->_color[1] = 0.0f;
          current_node->_color[2] = 0.0f;
        }

        inner_ring_node_weights_sum += current_node->getDegree();
      }

      // Draw inner ring
      radius = create_cluster_ring(inner_ring, globalPositionX, globalPositionY, 0);
      maxRadius = std::max(radius, maxRadius);

      // Create outer rings
      unsigned number_of_rings = 5; // Kann das berechnet werden?
      double step_width = (max_edge_weight - min_edge_weigth)/number_of_rings;

      // Draw outer rings
      for (unsigned i_ring = 0; i_ring != number_of_rings; i_ring++)
      {
        double ring_edge_weight_min = min_edge_weigth + (i_ring * step_width);
        double ring_edge_weight_max = min_edge_weigth + ((i_ring+1) * step_width);

        std::vector<Node*> current_ring;

        for (unsigned i_node = 0; i_node != outer_rings.size(); i_node++)
        {
          Node* current_node = outer_rings[i_node];
          double average_node_weight = get_average_edge_weight(current_node);

          if (average_node_weight >= ring_edge_weight_min && average_node_weight < ring_edge_weight_max) // Node liegt auf diesem Ring
          {
            current_ring.push_back(current_node);

            current_node->_color[0] = 0.0f;
            current_node->_color[1] = 0.0f;
            current_node->_color[2] = 0.1f * i_ring;
          }
        }

        if (current_ring.size() != 0)
        {
          double new_radius = create_cluster_ring(current_ring, globalPositionX, globalPositionY, i_ring);
          radius = std::max(radius, new_radius);
          maxRadius = std::max(radius, maxRadius);
        }
      }
    } // current_cluster.size() > 20

    else
    {
      // draw single ring clusters
      radius = create_cluster_ring(current_cluster, globalPositionX, globalPositionY, 0);
      maxRadius = std::max(radius, maxRadius);
    }


    globalPositionX += radius * 2.5;
    if (globalPositionX > 10000000.0)
    {
      globalPositionX = 10.0;
      globalPositionY += maxRadius + radius;
      maxRadius = 0.0;
    }
  }
}


/////////////////////////////////////////////////////////////////////////////////////////////////
// Radial Layout
/////////////////////////////////////////////////////////////////////////////////////////////////


void
Graph::make_radial_layout()
{
  double globalPositionX = 10.0;
  double globalPositionY = 10.0;

  double maxRadius = 0.0;

  for (unsigned i_cluster = 0; i_cluster != _all_clusters.size(); ++i_cluster)
  {
    std::vector<Node*> current_cluster = _all_clusters[i_cluster];

    double radius = 0.0;

    // Create multiple rings if cluster has more than 20 nodes
    if(current_cluster.size() > 20)
    {
      unsigned cluster_node_weights_sum = 0; // Number of edges of cluster
      double min_edge_weigth = 1.1; // Minimum edge weight of cluster
      double max_edge_weight = 0.0; // Maximum edge weight of cluster

      for(unsigned node_i = 0; node_i != current_cluster.size(); node_i++)
      {
        Node* current_node = current_cluster[node_i];

        cluster_node_weights_sum += current_node->getDegree();

        max_edge_weight = std::max(max_edge_weight, get_max_edge_weight(current_node));
        min_edge_weigth = std::min(min_edge_weigth, get_min_edge_weight(current_node));
      }

      // Fill inner ring and outer rings with nodes
      std::vector<Node*> inner_ring;
      std::vector<Node*> outer_ring;

      unsigned inner_ring_node_weights_sum = 0;
      for (unsigned node_i = 0; node_i != current_cluster.size(); node_i++)
      {
        Node* current_node = current_cluster[node_i];

        // if(node_i != 0) // Für 1 Node in der Mitte
        if(inner_ring_node_weights_sum >= cluster_node_weights_sum * 0.5)
        {
          outer_ring.push_back(current_node);

          current_node->_color[0] = 1.0f;
          current_node->_color[1] = 0.0f;
          current_node->_color[2] = 0.0f;
        }

        else
        {
          inner_ring.push_back(current_node);

          current_node->_color[0] = 0.0f;
          current_node->_color[1] = 0.0f;
          current_node->_color[2] = 1.0f;
        }

        inner_ring_node_weights_sum += current_node->getDegree();
      }

      // Draw inner ring
      radius = create_cluster_ring(inner_ring, globalPositionX, globalPositionY, 0);
      maxRadius = std::max(radius, maxRadius);

      // Draw outer ring
      radius = create_cluster_ring(outer_ring, globalPositionX, globalPositionY, 0);
      maxRadius = std::max(radius, maxRadius);

    } // current_cluster.size() > 20

    else
    {
      // draw single ring clusters
      radius = create_cluster_ring(current_cluster, globalPositionX, globalPositionY, 0);
      maxRadius = std::max(radius, maxRadius);
    }


    globalPositionX += radius * 2.5;
    if (globalPositionX > 1000000.0)
    {
      globalPositionX = 10.0;
      globalPositionY += maxRadius + radius;
      maxRadius = 0.0;
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Position nodes randomly
/////////////////////////////////////////////////////////////////////////////////////////////////

//void
//Graph::make_random_layout()
//{
//  std::srand(std::time(0)); // use current time as seed for random generator
//  float mult = 1.0f / RAND_MAX;
//
//  for (unsigned i = 0; i != _edges.size(); i++)
//  {
//    Edge* current_edge = _edges[i];
//    Node* source = current_edge->getSource();
//    Node* target = current_edge->getTarget();
//
//    source->_x = std::rand() * mult;;
//    source->_y = std::rand() * mult;;
//
//    target->_x = std::rand() * mult;;
//    target->_y = std::rand() * mult;;
//  }
//}
